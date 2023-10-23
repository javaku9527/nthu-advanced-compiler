#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include <llvm/IR/Instructions.h>
#include <llvm/Analysis/LoopInfo.h>
#include "llvm/Analysis/DependenceAnalysis.h"
#include <typeinfo>
#include <vector>
using namespace llvm;
using namespace std;

namespace
{
  class LoopVariables
  {
    StringRef name;
    vector<unsigned> operater;
    vector<int64_t> val;

    public:
    LoopVariables() {}

    void setName(StringRef name)
    {
      this->name = name;
    }

    StringRef getName()
    {
      return this->name;
    }

    void addArithmetic(unsigned opcode, int64_t val)
    {
      this->operater.push_back(opcode);
      this->val.push_back(val);
    }

    bool isSameArray(LoopVariables lv) 
    {
      return this->name.equals(lv.getName()); 
    }

    int cal(int idx) 
    {
      for(int i = 0; i < operater.size(); i++)
      {
        switch (operater[i]) 
        {
          case 13:
            idx += val[i];
            break;
          case 15:
            idx -= val[i];
            break;
          case 17:
            idx *= val[i];
            break;
        }
      }

      return idx;
    }
  };

 	//declare a “Hello” class that is a subclass of FunctionPass. 
  struct DataDependencyPass: public FunctionPass
  {
   	//pass identifier used by LLVM to identify pass
    static char ID;

    DataDependencyPass(): FunctionPass(ID) {}

    void handleArithmeticOperation(Value *operand, LoopVariables *lv, unsigned opcode)
    {
      if (ConstantInt *CI = dyn_cast<ConstantInt> (operand))
      {
        int64_t value = CI->getSExtValue();
        lv->addArithmetic(opcode, value);
      }
      else if (Instruction *x = dyn_cast<Instruction> (operand))
      {
        analysisInstruction(x, lv);
      }
    }

    void analysisInstruction(Instruction *Ins, LoopVariables *lv)
    {
      switch (Ins->getOpcode())
      {
        case Instruction::GetElementPtr:
          {
            StringRef arrayName = Ins->getOperand(0)->getName();
            lv->setName(arrayName);
            analysisInstruction(dyn_cast<Instruction> (Ins->getOperand(2)), lv);
            break;
          }

        case Instruction::Load:
        case Instruction::SExt:
          {
            if (Instruction *x = dyn_cast<Instruction> (Ins->getOperand(0)))
            {
              analysisInstruction(x, lv);
            }

            break;
          }

        case Instruction::Add:
        case Instruction::Mul:
        case Instruction::Sub:
          {
            handleArithmeticOperation(Ins->getOperand(0), lv, Ins->getOpcode());
            handleArithmeticOperation(Ins->getOperand(1), lv, Ins->getOpcode());
            break;
          }
      }
    }

    bool runOnFunction(Function & F) override
    {
      int state_num = 0;
      int64_t forStopCond;
      int64_t forStart = 0;
      vector<LoopVariables> leftVariables;
      vector<LoopVariables> rightVariables;

      for (Function::iterator b = F.begin(), be = F.end(); b != be; b++)
      {
        if (dyn_cast<BasicBlock> (b))
        {
            for (BasicBlock::iterator i = b->begin(); i != b->end(); i++)
            {
              if (Instruction *Ins = dyn_cast<Instruction> (i))
              {
                if (Ins->getOpcode() == Instruction::Store)
                {
                  if(Ins->getOperand(1)->hasName() & Ins->getOperand(1)->getName().startswith("arrayidx")) {
                   
                    LoopVariables *lv = new LoopVariables();
                    analysisInstruction(dyn_cast<Instruction> (Ins->getOperand(0)), lv);
                    rightVariables.push_back(*lv);
                    LoopVariables *lv2 = new LoopVariables();
                    analysisInstruction(dyn_cast<Instruction> (Ins->getOperand(1)), lv2);
                    leftVariables.push_back(*lv2);
                    state_num++;
                  }

                  if(Ins->getOperand(1)->hasName() & Ins->getOperand(1)->getName().equals("i")) {
                    if (ConstantInt *CI = dyn_cast<ConstantInt>(Ins->getOperand(0)))
                    {
                      forStart = CI->getSExtValue();
                    }
                  }
                }
                else if (Ins->getOpcode() == Instruction::ICmp)
                {
                  ConstantInt *CI = dyn_cast<ConstantInt> (Ins->getOperand(1));
                  forStopCond = CI->getSExtValue();
                }
              }
            }
        }
      }

      // 
      errs() << "====Flow Dependency====\n";
      for (int i = 0; i < leftVariables.size(); i++)
      {
        for (int x = 0; x < rightVariables.size(); x++) 
        {
          if(leftVariables[i].isSameArray(rightVariables[x]) && i !=x) 
          {
            for(int y = forStart; y < forStopCond ;y++ )
            {
              for(int z = y; z < forStopCond ;z++ )
              {
                if(leftVariables[i].cal(y) == rightVariables[x].cal(z)) 
                {
                  errs() << leftVariables[i].getName() << ":";
                  errs() << "S" << i + 1 << " ----->" << "S" << x + 1 << "\n";
                  errs() <<"(i=" << y << ",i=" << z << ")\n";
                }
              }
            }
          }
        }
      }

      errs() << "====Anti-Dependency====\n";
      for (int i = 0; i < leftVariables.size(); i++)
      {
        for (int x = 0; x < rightVariables.size(); x++) 
        {
          if(leftVariables[i].isSameArray(rightVariables[x]) && i !=x) 
          {
            for(int y = forStart; y < forStopCond ;y++ )
            {
              for(int z = y; z < forStopCond ;z++ )
              {
                if(leftVariables[i].cal(z) == rightVariables[x].cal(y)) 
                {
                  if(z == y && i <= x) {
                    break;
                  }
                  errs() << leftVariables[i].getName() << ":";
                  errs() << "S" << i + 1 << "--A-->" << "S" << x + 1 << "\n";
                  errs() <<"(i=" << y << ",i=" << z << ")\n";
                }
              }
            }
          }
        }
      }
      
      errs() << "====Output Dependency====\n";
      for (int i = 0; i < leftVariables.size(); i++)
      {
        for (int x = i; x < leftVariables.size(); x++) 
        {
          if(leftVariables[i].isSameArray(leftVariables[x]) && i !=x) 
          {
            for(int y = forStart; y < forStopCond ;y++ )
            {
              for(int z = forStart; z < forStopCond ;z++ )
              {
                bool isIndexEqual = leftVariables[i].cal(y) == leftVariables[x].cal(z);
                bool isIndex1Positive = leftVariables[i].cal(z) >= 0;
                bool isIndex2Positive = leftVariables[x].cal(y) >= 0;
                if( isIndexEqual && isIndex1Positive && isIndex2Positive ) 
                {
                  errs() << leftVariables[i].getName() << ":";
                  errs() << "S" << i + 1 << " --O-->" << "S" << x + 1 << "\n";
                  errs() <<"(i=" << y << ",i=" << z << ")\n";
                }
              }
            }
          }
        }
      }

      return false;
    }
  };

}

char DataDependencyPass::ID = 0;

//register class 
static RegisterPass<DataDependencyPass> X("hello", "Hello World Pass", false, false);