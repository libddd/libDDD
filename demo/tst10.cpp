#include <iostream>
using namespace std;
// for ::hash functions
using namespace __gnu_cxx;

#include "DDD.h"
#include "Hom.h"
#include "MemoryManager.h"

enum {A, B, C, D,E, F, G} variables;
char* vn[]= {"A", "B", "C", "D", "E", "F", "G"};

void initName() {
  for (int i=A; i<=G; i++)
    DDD::varName(i,vn[i]);
}

/// This Hom seeks a variable and applies its argument homomorphism when found.
class _seek:public StrongHom {
  /// the targeted var
  int var;
  /// the embedded homomorphism
  GHom h;
public :
  /// constructor
  /// \param vr the target variable's index
  /// \param hh the homomorphism to apply, either a simple hom, or a fixpoint construct
  _seek(int vr, const GHom & hh):var(vr),h(hh) {}

  GDDD phiOne() const {
    return GDDD::one;
  }                   

  GHom phi(int vr, int vl) const {
    if (vr == var)
      return h & GHom(vr,vl);
    else 
      return GHom(vr,vl,this); 
  }

  size_t hash() const {
    return var ^ ::hash<GHom>()(h);
  }

  bool operator==(const StrongHom &s) const {
    _seek* ps = (_seek*)&s;
    return var == ps->var && h==ps->h;
  }
};  

/// increment the value of the next variable
class _plusplus:public StrongHom {
public:
  _plusplus() {}

  GDDD phiOne() const {
    return GDDD::one;
  }                   

  GHom phi(int vr, int vl) const {
    return GHom(vr,vl+1);
  }

  size_t hash() const {
    return 23;
  }

  bool operator==(const StrongHom &s) const {
    return true;
  }
};

/// User function : Construct a Hom for a Strong Hom _plusplus
GHom plusplus(){return new _plusplus();};

/// selects only paths such that first occurrence of path has value < x.
/// i.e if next value on arc is strictly superior to lim, path is destroyed.
class _selectVarLim:public StrongHom {
  /// the limit tested against
  int lim;
public:
  /// constructor
  _selectVarLim(int vl):lim(vl) {}

  GDDD phiOne() const {
    return GDDD::one;
  }                   

  GHom phi(int vr, int vl) const {
      if (vl <= lim)
	return GHom(vr,vl);
      else
	return GDDD::null;
  }

  size_t hash() const {
    return lim*23;
  }

  bool operator==(const StrongHom &s) const {
    _selectVarLim* ps = (_selectVarLim*)&s;
    return lim == ps->lim;
  }
};


// User function : Construct a Hom for a Strong Hom _selectVarLim
GHom selectVarLim(int lim){return new _selectVarLim(lim);};

/// This example is built to show how superior saturation using fixpoint is to breadth first search.
/// It also gives a simple saturation example.
int main(){
  initName();
  
  
  
  cout <<"****************"<<endl;
  cout <<"* Define DDD u *"<<endl;
  cout <<"****************"<<endl;
  
  // This segment is a few variables long, it just increases the depth of the variable we want to work on (the last one).   
  DDD longSegment = DDD(A,1,DDD(A,1,DDD(A,1,DDD(A,1,DDD(A,1,DDD(A,1,DDD(A,1,DDD(A,1))))))));
  // an element in the example construct
  DDD a=DDD(A,1,DDD(A,1));  
  // an element in the example construct
  DDD b=DDD(B,1,DDD(C,1))+DDD(B,2,DDD(C,3));
  // an element in the example construct
  DDD c=DDD(A,1,DDD(A,2))^DDD(B,2,DDD(C,2)) ;
  // the value we work on
  DDD u=longSegment^( (a^b) + c );
	  
  cout <<"u="<< endl<<u<<endl;

  /// instantiate a plusplus hom
  Hom fc = plusplus();
  /// chooose the target limit
  Hom limc = selectVarLim(39);

  /// this block tests the fixpoint version
  {
  cout <<"**************************************************"<<endl;
  cout <<"* Fixpoint limit increment : increment all values of C *"<<endl;
  cout <<"* up to limit = 40, using an embedded fixpoint in saturation manner  *"<<endl;
  cout <<"**************************************************"<<endl;

  Hom satLim = fixpoint( GHom::id + ( fc & limc) );
  Hom full = GHom(new _seek(C,satLim));

  cerr <<"<!C++<=6>(u)="<< endl<<full(u).nbStates()<<endl;
  }
  /// print and clear stats for next run
  DDD::pstats(true);
  DED::pstats(true);
  MemoryManager::garbage();

  /// this block shows a BFS version
  {
  cout <<"**************************************************"<<endl;
  cout <<"* BFS limit increment : increment all values of C *"<<endl;
  cout <<"* up to limit = 40, using a naive \"iterate until fixpoint\" strategy *"<<endl;
  cout <<"**************************************************"<<endl;

  Hom full = GHom(new _seek(C,fc & limc));

  /// the external fixpoint iteration
  DDD v = u;
  do {
    v = u;
    u = u + full (u);
  }  while (u!=v) ;
  
  cerr <<"<!C++<=6>(u)="<< endl<<u.nbStates()<<endl;
  }
  /// show stats
  DDD::pstats(true);
  DED::pstats(true);
  MemoryManager::garbage();

  return 1;

}
