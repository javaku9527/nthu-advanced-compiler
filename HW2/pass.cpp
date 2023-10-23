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
  class EquivVector
  {
    vector<pair<string, string>> equivalents;

    void removeDuplicate(pair<string, string> equ) {
      auto it = equivalents.begin();
      while (it != equivalents.end()) {
        if ((*it).first == equ.first)
          it = equivalents.erase(it);
        else
          ++it;
      }
    }

    void push_new(pair<string, string> equ) {
      removeDuplicate(equ);
      equivalents.push_back(equ);
    }

    public:
    void print() {
      errs() << "TEQUIV: {";
      auto it = std::begin(equivalents);
      while (it != std::end(equivalents)) {
        if(it != equivalents.end() - 1)
          errs() << "(" << (*it).first << ", " << (*it).second << "), ";
        else
          errs() << "(" << (*it).first << ", " << (*it).second << ")";
        ++it;
      }
      errs() << "}\n";
    }     

    void push(pair<string, string> *equ) {
      
      push_new(*equ);
      for(int i = 0; i < equivalents.size(); i++) {
        string s = "*" + equivalents[i].first;
        for(int x = 0; x < equivalents.size(); x++) {
          if(i != x && s == equivalents[x].first) {
            pair<string,string> p("*" + equivalents[i].second, equivalents[x].second);
            push_new(p);
          }
        }
      }

      for(int i = 0; i < equivalents.size(); i++) {
        string s = "*" + equivalents[i].second;
        for(int x = 0; x < equivalents.size(); x++) {
          if(i != x && s == equivalents[x].first) {
            pair<string,string> p("*" + equivalents[i].first, equivalents[x].second);
            push_new(p);
          }
        }
      }
    }

    vector<string> getEquis(string s) {
      vector<string> equis;
      auto it = equivalents.begin();
      while (it != equivalents.end()) {
        if ((*it).first == s)
          equis.push_back((*it).second);
        else if((*it).second == s)
          equis.push_back((*it).first);
        it++;
      }

      return equis;
    }
  };

  class Definition
  {
    public:
    string name;
    int line;
    
    Definition(string name, int line) {
      this->name = name;
      this->line = line;
    }
  };

  class DefinitionVector
  {
    vector<Definition> definitions;

    public:
    DefinitionVector() {}

    void push(Definition *def) {
      auto it = std::begin(definitions);
      while (it != std::end(definitions)) {
          if ((*it).name == def->name)
            it = definitions.erase(it);
          else
            ++it;
      }
      definitions.push_back(*def);
    }

    int size() {
      return definitions.size();
    }

    void print() {
      errs() << "TDEF: {";
      auto it = std::begin(definitions);
      while (it != std::end(definitions)) {
        if(it != definitions.end() - 1)
          errs() << "(" << (*it).name << ", S" << (*it).line << "), ";
        else
          errs() << "(" << (*it).name << ", S" << (*it).line << ")";
        ++it;
      }
      errs() << "}\n";
    }

    void findDependency(string s, bool b, int l) {
      auto it = std::begin(definitions);
      while (it != std::end(definitions)) {
        if ((*it).name == s && b)
          errs() << "\n" << s << ": S" << (*it).line << "--->S" << l;
        else if((*it).name == s && !b)
          errs() << "\n" << s << ": S" << (*it).line << "-O->S" << l;
        it++;
      }
    }
  };

 	//declare a “Hello” class that is a subclass of FunctionPass. 
  struct DataDependencyPass: public FunctionPass
  {
   	//pass identifier used by LLVM to identify pass
    static char ID;
    EquivVector *equivalents = new EquivVector();
    DefinitionVector *definitions = new DefinitionVector();
    vector<string> gens;
    vector<string> refs;

    DataDependencyPass(): FunctionPass(ID) {}

    void genVarName(string &name, StringRef var, int n) {       
      name = std::string(n, '*') + (string)var;
    }

    void addEqui() {
      vector<string> quis;
      for(int i = 0; i < gens.size(); i++) {
        vector<string> vector = equivalents->getEquis(gens[i]);
        quis.insert(quis.end(), vector.begin(), vector.end());    
      }

      for(int x = 0; x < quis.size(); x++) {
        if(find(gens.begin(), gens.end(), quis[x]) == gens.end()) {
          gens.push_back(quis[x]);
        }
      }

      quis.clear();
      for(int i = 0; i < refs.size(); i++) {
        vector<string> vector = equivalents->getEquis(refs[i]);
        quis.insert(quis.end(), vector.begin(), vector.end());    
      }

      for(int x = 0; x < quis.size(); x++) {
        if(find(refs.begin(), refs.end(), quis[x]) == refs.end()) {
          refs.push_back(quis[x]);
        }
      }

    }

    void addDefinition(int lineNum) {
      for(int i = 0; i< gens.size(); i++) {
        definitions->push(new Definition(gens[i], lineNum));
      }
    }

    void printDep(int lineNum) {
      errs() << "DEP: {";
      for(int i = 0; i < refs.size(); i++) {
        definitions->findDependency(refs[i], true, lineNum);
      }
      for(int i = 0; i < gens.size(); i++) {
        definitions->findDependency(gens[i], false, lineNum);
      }
      errs() << "\n}\n";
    }
    
    void printGen() {
      errs() << "TGEN: {";
      auto it = std::begin(gens);
      while (it != std::end(gens)) {
        if(it != gens.end() - 1)
          errs() << *it << ", ";
        else
          errs() << *it;
        ++it;
      }
      errs() << "}\n";
    }

    void printRef() {
      errs() << "TREF: {";
      auto it = std::begin(refs);
      while (it != std::end(refs)) {
        if(it != refs.end() - 1)
          errs() << *it << ", ";
        else
          errs() << *it;
        ++it;
      }
      errs() << "}\n";
    }

    void rec(Instruction *Ins)
    {
      switch (Ins->getOpcode())
      {
        case Instruction::Alloca:
        {
          refs.push_back(Ins->getName());
        }
        case Instruction::Load:
        {
          if(Ins->getOperand(0)->hasName()) {
            refs.push_back(Ins->getOperand(0)->getName());
          }
          break;
        }
        case Instruction::Add:
        case Instruction::Mul:
        case Instruction::Sub:
        {
          if(Instruction *left = dyn_cast<Instruction>(Ins->getOperand(0)))
            rec(left);
          if(Instruction *right = dyn_cast<Instruction>(Ins->getOperand(1)))
            rec(right);
          break;
        }
      }
    }

    void analysisRHS(Value *x) 
    {
      if (Instruction *Ins = dyn_cast<Instruction>(x)) {
        if(!Ins->getType()->isPointerTy()) {
          rec(Ins);
        } else {
          // errs() << Ins->getName();
          for(int i = 0; i < gens.size(); i++) {
            pair<string,string> p("*" +gens[i], Ins->getName());
            equivalents->push(&p);
          }
        }
      }
    }

    void analysisLHS(Instruction *Ins)
    {
      string name;
      if(Ins->hasName()) {
        name = Ins->getName();
      } else {
        switch (Ins->getOpcode())
        {
          case Instruction::Load:
            if( Ins->getType()->isPointerTy()) {
              Instruction *x = Ins;
              int i = 1;
              while(!x->getOperand(0)->hasName()) {
                x = dyn_cast<Instruction> (x->getOperand(0));
                genVarName(name, x->getOperand(0)->getName(), i);
                refs.push_back(name);
                i++;
              }
              refs.push_back(x->getOperand(0)->getName());
              genVarName(name, x->getOperand(0)->getName(), i);
            }
            else
             name = Ins->getOperand(0)->getName();
        }
      }
      gens.push_back(name);
    }

    bool runOnFunction(Function & F) override
    {
      int state_num = 1;

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
                  refs.clear();
                  gens.clear();
                  analysisLHS(dyn_cast<Instruction>(Ins->getOperand(1)));
                  analysisRHS(Ins->getOperand(0));
                  addEqui();

                  errs() << "\nS" << state_num << ":--------------------\n";
                  printRef();
                  printGen();
                  printDep(state_num);
                  addDefinition(state_num);
                  definitions->print();
                  equivalents->print();
                  state_num++;
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