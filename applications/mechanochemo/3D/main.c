#include <math.h> 
extern "C" {
#include "petiga.h"
}

//model parameters
#define DIM 3
#define GridScale 1.0
//#define ADSacado //enables Sacado for AD instead of Adept
#define numVars 108

//generic headers
#include "../../../include/fields.h"
#include "../../../include/appctx.h"
#include "../../../include/evaluators.h"
#include "../../../include/init.h"

//physical parameters
#define Cs 1.0
#define Cd -1.0
#define C4 (-16*Cd/std::pow(Cs,4))
#define C3 (32*Cd/std::pow(Cs,3))
#define C2 (-16*Cd/std::pow(Cs,2))
#define Es 0.01
#define Ed -0.001
#define E4 (-3*Ed/(2*std::pow(Es,4)))
#define E3 (Ed/(std::pow(Es,3)))*(c/Cs)
#define E2 (3*Ed/(2*std::pow(Es,2)))*(2*c/Cs-1.0)
#define E4_c 0.0
#define E3_c (Ed/(std::pow(Es,3)))*(1.0/Cs)
#define E2_c (3*Ed/(2*std::pow(Es,2)))*(2.0/Cs)
#define E4_cc 0.0
#define E3_cc 0.0
#define E2_cc 0.0
#define Eii (-1.5*Ed/std::pow(Es,2))
#define Eij (-1.5*Ed/std::pow(Es,2))
#define Cl  (0.01*GridScale*GridScale) //Clambda - constant for gradC.gradC
#define El 0.01000 //**ELambda - constant for gradE.gradE
#define Gl 0.0  //Glambda - constant for abs(gradC.gradE)...this is not fully implemented in 3D as requires C^2 space
//stress
#define PiJ (2*Eii*e1*e1_FiJ + 2*Eij*e4*e4_FiJ + 2*Eij*e5*e5_FiJ + 2*Eij*e6*e6_FiJ + (2*E2*e2-6*E3*e2*e3+4*E4*e2*(e2*e2+e3*e3))*e2_FiJ + (2*E2*e3+3*E3*(e3*e3-e2*e2)+4*E4*e3*(e2*e2+e3*e3))*e3_FiJ + 2*El*(e2_1*e2_1_FiJ + e2_2*e2_2_FiJ + e2_3*e2_3_FiJ + e3_1*e3_1_FiJ + e3_2*e3_2_FiJ + e3_3*e3_3_FiJ))
#define BetaiJK  2*El*(e2_1*e2_1_FiJK + e2_2*e2_2_FiJK + e2_3*e2_3_FiJK + e3_1*e3_1_FiJK + e3_2*e3_2_FiJK + e3_3*e3_3_FiJK)
//chemistry
#define mu_c (12*C4*c*c+6*C3*c+2*C2) // E4_cc, E3_cc, E2_cc are zero so skipping those terms 
#define mu_e2 (4*E4_c*e2*(e2*e2+e3*e3) - 6*E3_c*e3*e2 + 2*E2_c*e2)
#define mu_e3 (4*E4_c*e3*(e2*e2+e3*e3) + 3*E3_c*(e3*e3-e2*e2) + 2*E2_c*e3)
//boundary conditions
#define bcVAL 3 //**
#define FLUX 0 //**
#define flux 0.0
#define uDirichlet 0.01
//other variables
#define NVal 80 //**
#define DVal (0*10.0*GridScale*GridScale)
#define CVal 5.0
#define gamma 1.0
//time stepping
#define dtVal 1.0e-5 //**
#define skipOutput 1

//physics headers
#include "../../../src/mechanochemo/model.h"
#include "../../../src/mechanochemo/initialConditions.h"
#include "../../../src/mechanochemo/output.h"

//snes convegence test
PetscErrorCode SNESConvergedTest(SNES snes, PetscInt it,PetscReal xnorm, PetscReal snorm, PetscReal fnorm, SNESConvergedReason *reason, void *ctx){
  AppCtx *user  = (AppCtx*) ctx;
  //custom test
  if (NVal>=15){
    if (it>50){
      *reason = SNES_CONVERGED_ITS;
      return(0);
    }
  }
  //default test
  PetscFunctionReturn(SNESConvergedDefault(snes,it,xnorm,snorm,fnorm,reason,ctx));
}

int main(int argc, char *argv[]) {
  PetscErrorCode  ierr;
  ierr = PetscInitialize(&argc,&argv,0,0);CHKERRQ(ierr);
 
  //application context objects and parameters
  AppCtx user;
  user.dt=dtVal;
  user.he=GridScale*1.0/NVal; 
  PetscInt p=2;

  //initialize
  IGA iga;
  Vec U,U0;
  user.iga = iga;
  user.U0=&U0;
  user.U=&U;

  PetscPrintf(PETSC_COMM_WORLD,"initializing...\n");
  init(user, NVal, p, DIM+1);

  //Dirichlet boundary conditons for mechanics
  PetscPrintf(PETSC_COMM_WORLD,"applying bcs...\n");
  double dVal=uDirichlet*GridScale;
#if bcVAL==0
  //shear BC
  ierr = IGASetBoundaryValue(iga,0,0,1,dVal);CHKERRQ(ierr);  
  ierr = IGASetBoundaryValue(iga,0,1,1,-dVal);CHKERRQ(ierr);  
  ierr = IGASetBoundaryValue(iga,1,0,0,dVal);CHKERRQ(ierr);  
  ierr = IGASetBoundaryValue(iga,1,1,0,-dVal);CHKERRQ(ierr);  
  ierr = IGASetBoundaryValue(iga,2,0,2,0.0);CHKERRQ(ierr);  
#elif bcVAL==1
  //free BC
  ierr = IGASetBoundaryValue(user.iga,0,0,0,0.0);CHKERRQ(ierr);  
  ierr = IGASetBoundaryValue(user.iga,1,0,1,0.0);CHKERRQ(ierr);
  ierr = IGASetBoundaryValue(user.iga,2,0,2,0.0);CHKERRQ(ierr);
  ierr = IGASetBoundaryValue(user.iga,0,1,0,dVal);CHKERRQ(ierr);  
#elif bcVAL==2
  //fixed BC
  ierr = IGASetBoundaryValue(user.iga,0,0,0,0.0);CHKERRQ(ierr);  
  ierr = IGASetBoundaryValue(user.iga,0,0,1,0.0);CHKERRQ(ierr);  
  ierr = IGASetBoundaryValue(user.iga,0,0,2,0.0);CHKERRQ(ierr);  
  ierr = IGASetBoundaryValue(user.iga,0,1,1,dVal);CHKERRQ(ierr);  
  ierr = IGASetBoundaryValue(user.iga,0,1,2,dVal);CHKERRQ(ierr);  
#elif bcVAL==3
  //hydrostatic like BC
  ierr = IGASetBoundaryValue(user.iga,0,0,0,0.0);CHKERRQ(ierr);  
  ierr = IGASetBoundaryValue(user.iga,1,0,1,0.0);CHKERRQ(ierr);  
  ierr = IGASetBoundaryValue(user.iga,2,0,2,0.0);CHKERRQ(ierr);  
  ierr = IGASetBoundaryValue(user.iga,0,1,0,dVal);CHKERRQ(ierr);  
  ierr = IGASetBoundaryValue(user.iga,1,1,1,dVal);CHKERRQ(ierr);  
  ierr = IGASetBoundaryValue(user.iga,2,1,2,dVal);CHKERRQ(ierr);  
#endif 

  //initial conditions
  ierr = IGACreateVec(user.iga,user.U);CHKERRQ(ierr);
  ierr = IGACreateVec(user.iga,user.U0);CHKERRQ(ierr);
  ierr = FormInitialCondition(user.iga, *user.U0, &user); 
  ierr = VecCopy(*user.U0, *user.U);CHKERRQ(ierr);

  //time stepping
  TS ts;
  ierr = IGACreateTS(user.iga,&ts);CHKERRQ(ierr);
  ierr = TSSetType(ts,TSBEULER);CHKERRQ(ierr);
  ierr = TSSetDuration(ts,100000,1.0);CHKERRQ(ierr);
  ierr = TSSetTime(ts,0.0);CHKERRQ(ierr);
  ierr = TSSetTimeStep(ts,user.dt);CHKERRQ(ierr);
  ierr = TSMonitorSet(ts,OutputMonitor<DIM>,&user,NULL);CHKERRQ(ierr);  
  ierr = TSSetFromOptions(ts);CHKERRQ(ierr);

  //set snes convergence tests
  SNES snes;
  ierr = TSGetSNES(ts,&snes); CHKERRQ(ierr);
  ierr = SNESSetConvergenceTest(snes,SNESConvergedTest,&user,NULL); CHKERRQ(ierr);

  //run
  PetscPrintf(PETSC_COMM_WORLD,"running...\n");
  user.ts=&ts;
  ierr = TSSolve(ts,*user.U);CHKERRQ(ierr);

  //finalize
  ierr = TSDestroy(&ts);CHKERRQ(ierr);
  ierr = SNESDestroy(&snes);CHKERRQ(ierr);
  ierr = VecDestroy(user.U);CHKERRQ(ierr);
  ierr = VecDestroy(user.U0);CHKERRQ(ierr);
  ierr = IGADestroy(&user.iga);CHKERRQ(ierr);
  
  ierr = PetscFinalize();CHKERRQ(ierr);
  return 0;
}
