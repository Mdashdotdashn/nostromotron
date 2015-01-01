
// Linear regression
//
// See http://onlinestatbook.com/2/regression/intro.html
//
// 
// inpired from http://www.c-program-example.com/2011/11/c-program-to-implement-linear_17.html

#include <math.h>
#include <string.h>

//-----------------------------------------------------------

static float SCalcMean(const float *a, size_t n) 
{
   float sum = 0;
   for (size_t i = 0; i < n; i++)
   {
     sum = sum + a[i];
   }
   sum = sum / n;
   return (sum);
}


//-----------------------------------------------------------
 
static void SCalcDeviation(const float *a, const float mean, const int n, float *d, float *s)
{
   float sum = 0, t = 0;
   int i = 0;
   for (i = 0; i < n; i++) 
   {
     d[i] = a[i] - mean;
     t = d[i] * d[i];
     sum = sum + t;
   }
   sum = sum / n;
   *s = sqrt(sum);
}


//-----------------------------------------------------------

void SLinearRegression(const float* x, const float *y, const size_t size, float* slope, float* intercept)
{
  float mean_x = SCalcMean(x, size);
  float mean_y = SCalcMean(y, size);
  
  float sx = 0.f ;
  float sy = 0.f;
  float dx[size], dy[size];
  
  SCalcDeviation(x, mean_x, size, dx, &sx);
  SCalcDeviation(y, mean_y, size, dy, &sy);
  
  float sum_xy = 0;
  
  for (size_t i = 0; i < size; i++)
  {
    sum_xy = sum_xy + dx[i] * dy[i];
  }

  float corr_coff = sum_xy / (size * sx * sy);  
  
  *slope = corr_coff * (sy / sx);
  *intercept = mean_y - (*slope) * mean_x;
}
