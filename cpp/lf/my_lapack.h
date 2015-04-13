#ifndef __MY_LAPACK_H__
#define __MY_LAPACK_H__

#ifdef LAPACK


#ifdef __UNIX__
#define DAXPY daxpy_
#define DNRM2 dnrm2_
#define DSCAL dscal_
#define DDOT ddot_
#define DSYEV dsyev_
#define DSYEVX dsyevx_
#define DGESV dgesv_
#define DGESVD dgesvd_
#define DGELSS dgelss_
/*#else
#ifdef _WIN32
#define DAXPY daxpy
#define DNRM2 dnrm2
#define DSCAL dscal
#define DDOT ddot
#define DSYEV dsyev_
#define DSYEVX dsyevx
#endif*/
#endif


extern "C"
{
  // -------------------- BLAS routines --------------------
  void DAXPY(const int *, const double *, const double *, const int *, 
	     double *, const int *);
  double DNRM2(const int *, const double *, const int *);
  double DDOT(const int *, const double *, const int *, const double *, 
	      const int *);  

  // -------------------- LAPACK routines --------------------

  int DSYEV(char *jobz, char *uplo, int *n, double *a,
	     int *lda, double *w, double *work, int *lwork, 
	     int *info);

  int DSYEVX(char *jobz, char *range, char *uplo, int *n, 
	     double *a, int *lda, double *vl, double *vu, int *
	     il, int *iu, double *abstol, int *m, double *w, 
	     double *z__, int *ldz, double *work, int *lwork, 
	     int *iwork, int *ifail, int *info);

  int DGESV(int *n, int *n, double *A, int *n, int * ipiv,
	    double *B, int * ldb, int * info);

  int DGESVD(char * jobu, char * jobvt, int * m, int * n,
	     double * A, int * lda, double * s, double * u,
	     int * ldu, double * vt,int * ldvt, double * work,
	     int * lwork, int * info);

  int DGELSS(int * m, int * n, int * nrhs, double * A, int * lda,
	     double * B, int * ldb, double * s,double * rcond,int * rank,
	     double * work, int * lwork, int *info);
}
#endif



#endif
