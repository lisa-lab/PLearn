// -*- C++ -*-
// convolutions.cc

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of
// Montreal
//

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//
//  3. The name of the authors may not be used to endorse or promote
//     products derived from this software without specific prior written
//     permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
// NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// This file is part of the PLearn library. For more information on the PLearn
// library, go to the PLearn Web site at www.plearn.org


/* *******************************************************
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */

/*! \file plearn/math/convolutions.cc */


#include "convolutions.h"

namespace PLearn {
using namespace std;

void convolve1D(const Vec& source_signal, const Vec& kernel,
                const Vec& dest_signal, int step, bool accumulate)
{
    int nk=kernel.length();
    int nd=dest_signal.length();
#ifdef BOUNDCHECK
    int ns=source_signal.length();
    if (step<1)
        PLERROR("convolve1D: step (%d) should be a positive integer\n",step);
    if (ns!=step*(nd-1)+nk)
        PLERROR("convolve1D: source_signal.length() (%d) should equal %d:\n"
                "step (%d) * (dest_signal.length() (%d) - 1) + kernel.length()"
                " (%d)\n",
                ns,step*(nd-1)+nk,step,nd,nk);
#endif
    if (!accumulate)
        dest_signal.clear();
    real* s=source_signal.data();
    real* k=kernel.data();
    real* d=dest_signal.data();
    for (int i=0;i<nd;i++,s+=step)
    {
        real somme=0;
        for (int j=0;j<nk;j++)
            somme += s[j]*k[j];
        d[i]+=somme;
    }
}

void backConvolve1D(const Vec& source_signal, const Vec& kernel,
                    const Vec& dest_signal, int step, bool accumulate)
{
    int nk=kernel.length();
    int nd=dest_signal.length();
#ifdef BOUNDCHECK
    int ns=source_signal.length();
    if (step<1)
        PLERROR("backConvolve1D: step (%d) should be a positive integer\n",
                step);
    if (ns!=step*(nd-1)+nk)
        PLERROR("backConvolve1D: source_signal.length() (%d) should equal %d:\n"
                "step (%d) * (dest_signal.length() (%d) - 1) + kernel.length()"
                " (%d)\n",
                ns,step*(nd-1)+nk,step,nd,nk);
#endif
    if (!accumulate)
        source_signal.clear();
    real* s=source_signal.data();
    real* k=kernel.data();
    real* d=dest_signal.data();
    for (int i=0;i<nd;i++,s+=step)
    {
        //! for i=0 to nd-1:
        //!   for j=0 to nk-1:
        //!    source_signal[i*step+j] += dest_signal[i]*kernel[j]
        real di=d[i];
        for (int j=0;j<nk;j++)
            s[j] += di*k[j];
    }
}


void convolve1Dbackprop(const Vec& source_signal, const Vec& kernel,
                        const Vec& dC_ddest_signal,
                        const Vec& dC_dsource_signal, const Vec& dC_dkernel,
                        int step, bool accumulate)
{
    int nk=kernel.length();
    int nd=dC_ddest_signal.length();
#ifdef BOUNDCHECK
    int ns=source_signal.length();
    if (step<1)
        PLERROR("convolve1Dbackprop: step (%d) should be a positive integer\n",
                step);
    if (ns!=step*(nd-1)+nk)
        PLERROR("convolve1Dbackprop: source_signal.length() (%d) should"
                " equal %d:\n"
                "step (%d) * (dC_ddest_signal.length() (%d) - 1) +"
                " kernel.length() (%d)\n",
                ns,step*(nd-1)+nk,step,nd,nk);
    if (dC_dsource_signal.length()!=ns)
        PLERROR("convolve1Dbackprop: source_signal.length() (%d) should"
                " equal:\n"
                "dC_dsource_signal.length() (%d)\n",
                ns,dC_dsource_signal.length());
    if (dC_dkernel.length()!=nk)
        PLERROR("convolve1Dbackprop: kernel.length() (%d) should equal:\n"
                " dC_dkernel.length() (%d)\n",
                nk,dC_dkernel.length());
#endif
    if (!accumulate)
    {
        dC_dsource_signal.clear();
        dC_dkernel.clear();
    }
    real* s=source_signal.data();
    real* dCds=dC_dsource_signal.data();
    real* k=kernel.data();
    real* dCdk=dC_dkernel.data();
    real* dCdd=dC_ddest_signal.data();

    //! for i=0 to nd-1:
    //!   for j=0 to nk-1:
    //!     dC_dsource_signal[i*step+j] += dC_ddest_signal[i]*kernel[j]
    //!     dC_dkernel[j] += dC_ddest_signal[i]*source_signal[i*step+j]
    for (int i=0;i<nd;i++,dCds+=step,s+=step)
    {
        real di=dCdd[i];
        for (int j=0;j<nk;j++)
        {
            dCds[j] += di*k[j];
            dCdk[j] += di*s[j];
        }
    }
}

void convolve1Dbackprop(const Vec& source_signal,
                        const Vec& dC_ddest_signal,
                        const Vec& dC_dkernel,
                        int step, bool accumulate)
{
    int nk=dC_dkernel.length();
    int nd=dC_ddest_signal.length();
#ifdef BOUNDCHECK
    int ns=source_signal.length();
    if (step<1)
        PLERROR("convolve1Dbackprop: step (%d) should be a positive integer\n",
                step);
    if (ns!=step*(nd-1)+nk)
        PLERROR("convolve1Dbackprop: source_signal.length() (%d) should"
                " equal %d:\n"
                "step (%d) * (dC_ddest_signal.length() (%d) - 1) +"
                " dC_dkernel.length() (%d)\n",
                ns,step*(nd-1)+nk,step,nd,nk);
#endif
    if (!accumulate)
        dC_dkernel.clear();

    real* s=source_signal.data();
    real* dCdk=dC_dkernel.data();
    real* dCdd=dC_ddest_signal.data();

    //! for i=0 to nd-1:
    //!   for j=0 to nk-1:
    //!     dC_dkernel[j] += dC_ddest_signal[i]*source_signal[i*step+j]
    for (int i=0;i<nd;i++,s+=step)
    {
        real di=dCdd[i];
        for (int j=0;j<nk;j++)
            dCdk[j] += di*s[j];
    }
}


void backConvolve1Dbackprop(const Vec& kernel, const Vec& dest_signal,
                            const Vec& dC_ddest_signal,
                            const Vec& dC_dsource_signal,
                            const Vec& dC_dkernel,
                            int step, bool accumulate)
{
    int nk=kernel.length();
    int nd=dest_signal.length();
#ifdef BOUNDCHECK
    int ns=dC_dsource_signal.length();
    if (step<1)
        PLERROR("backConvolve1Dbackprop: step (%d) should be a positive"
                " integer\n",
                step);
    if (ns!=step*(nd-1)+nk)
        PLERROR("backConvolve1Dbackprop: dC_dsource_signal.length() (%d)\n"
                "should equal %d:\n"
                "step (%d) * (dest_signal.length() (%d) - 1) + kernel.length()"
                " (%d)\n",
                ns,step*(nd-1)+nk,step,nd,nk);
    if (dC_ddest_signal.length()!=nd)
        PLERROR("backConvolve1Dbackprop: dest_signal.length() (%d) should"
                " equal:\n"
                "dC_ddest_signal.length() (%d)\n",
                nd,dC_ddest_signal.length());
    if (dC_dkernel.length()!=nk)
        PLERROR("backConvolve1Dbackprop: kernel.length() (%d) should equal:\n"
                " dC_dkernel.length() (%d)\n",
                nk,dC_dkernel.length());
#endif
    if (!accumulate)
    {
        dC_ddest_signal.clear();
        dC_dkernel.clear();
    }
    real* k=kernel.data();
    real* dCdk=dC_dkernel.data();
    real* dCdd=dC_ddest_signal.data();
    real* d=dest_signal.data();
    real* dCds=dC_dsource_signal.data();

    //! for i=0 to nd-1:
    //!   for j=0 to nk-1:
    //!     dC_ddest_signal[i] += dC_dsource_signal[i*step+j]*kernel[j]
    //!     dC_dkernel[j] += dC_dsource_signal[i*step+j]*dest_signal[i]
    for (int i=0;i<nd;i++,dCds+=step)
    {
        real di = d[i];
        real dCdd_i_sum = 0;
        for (int j=0;j<nk;j++)
        {
            dCdd_i_sum += dCds[j]*k[j];
            dCdk[j] += dCds[j]*di;
        }
        dCdd[i] += dCdd_i_sum;
    }
}


void backConvolve1Dbackprop(const Vec& dest_signal,
                            const Vec& dC_dsource_signal,
                            const Vec& dC_dkernel,
                            int step, bool accumulate)
{
    int nk=dC_dkernel.length();
    int nd=dest_signal.length();
#ifdef BOUNDCHECK
    int ns=dC_dsource_signal.length();
    if (step<1)
        PLERROR("backConvolve1Dbackprop: step (%d) should be a positive"
                " integer\n",
                step);
    if (ns!=step*(nd-1)+nk)
        PLERROR("backConvolve1Dbackprop: dC_dsource_signal.length() (%d)\n"
                "should equal %d:\n"
                "step (%d) * (dest_signal.length() (%d) - 1) + dC_dkernel.length()"
                " (%d)\n",
                ns,step*(nd-1)+nk,step,nd,nk);
#endif
    if (!accumulate)
        dC_dkernel.clear();

    real* dCdk=dC_dkernel.data();
    real* d=dest_signal.data();
    real* dCds=dC_dsource_signal.data();

    //! for i=0 to nd-1:
    //!   for j=0 to nk-1:
    //!     dC_dkernel[j] += dC_dsource_signal[i*step+j]*dest_signal[i]
    for (int i=0;i<nd;i++,dCds+=step)
    {
        real di = d[i];
        for (int j=0;j<nk;j++)
            dCdk[j] += dCds[j]*di;
    }
}


void convolve2D(const Mat& source_image, const Mat& kernel,
                const Mat& dest_image,
                int step1, int step2, bool accumulate)
{
    int kl = kernel.length();
    int kw = kernel.width();
    int dl = dest_image.length();
    int dw = dest_image.width();

#ifdef BOUNDCHECK
    int sl = source_image.length();
    int sw = source_image.width();

    if (step1<1)
        PLERROR("convolve2D: step1 (%d) should be a positive integer\n",step1);
    if (sl != step1*(dl-1)+kl)
        PLERROR("convolve2D: source_image.length() (%d) should equal %d:\n"
                "step1 (%d) * (dest_image.length() (%d) - 1) + kernel.length()"
                " (%d)\n",
                sl, step1*(dl-1)+kl, step1, dl, kl);

    if (step2<1)
        PLERROR("convolve2D: step2 (%d) should be a positive integer\n",step2);
    if (sw != step2*(dw-1)+kw)
        PLERROR("convolve2D: source_image.width() (%d) should equal %d:\n"
                "step2 (%d) * (dest_image.width() (%d) - 1) + kernel.width()"
                " (%d)\n",
                sw, step2*(dw-1)+kw, step2, dw, kw);
#endif
    if (!accumulate)
        dest_image.clear();
    int sm = source_image.mod();
    int dm = dest_image.mod();
    int km = kernel.mod();
    real* source_i = source_image.data(); // source_image[i*step1]
    real* dest_i = dest_image.data(); // dest_image[i]
    for (int i=0; i<dl; i++, source_i+=sm*step1, dest_i+=dm)
    {
        real* source_i_j = source_i; // source_image[i*step1][j*step2]
        for (int j=0; j<dw; j++, source_i_j+=step2)
        {
            real sum = 0;
            real* kernel_k = kernel.data(); // kernel[k]
            real* source_ik_j = source_i_j; // source_image[i*step1+k][j*step2]
            for (int k=0; k<kl; k++, source_ik_j+=sm, kernel_k+=km)
                for (int l=0; l<kw; l++)
                    sum += source_ik_j[l] * kernel_k[l];
            dest_i[j] += sum;
        }
    }
}

void backConvolve2D(const Mat& source_image, const Mat& kernel,
                    const Mat& dest_image,
                    int step1, int step2, bool accumulate)
{
    int n1k=kernel.length();
    int n2k=kernel.width();
    int n1d=dest_image.length();
    int n2d=dest_image.width();
#ifdef BOUNDCHECK
    int n1s=source_image.length();
    int n2s=source_image.width();
    if (step1<1)
        PLERROR("backConvolve2D: step1 (%d) should be a positive integer\n",
                step1);
    if (n1s!=step1*(n1d-1)+n1k)
        PLERROR("backConvolve2D: source_image.length() (%d) should equal %d:\n"
                "step1 (%d) * (dest_image.length() (%d) - 1) + kernel.length()"
                " (%d)\n",
                n1s,step1*(n1d-1)+n1k,step1,n1d,n1k);

    if (step2<1)
        PLERROR("backConvolve2D: step2 (%d) should be a positive integer\n",
                step2);
    if (n2s!=step2*(n2d-1)+n2k)
        PLERROR("backConvolve2D: source_image.width() (%d) should equal %d:\n"
                "step2 (%d) * (dest_image.width() (%d) - 1) + kernel.width()"
                " (%d)\n",
                n2s,step2*(n2d-1)+n2k,step2,n2d,n2k);
#endif
    if (!accumulate)
        source_image.clear();
    int sm = source_image.mod();
    int dm = dest_image.mod();
    int km = kernel.mod();
    real* s = source_image.data();
    real* d = dest_image.data();
    for (int i=0;i<n1d;i++,s+=sm*step1,d+=dm)
    {
        real* s1 = s; // copy to iterate over columns
        for (int j=0;j<n2d;j++,s1+=step2)
        {
            real* k = kernel.data();
            real* ss = s1; // copy to iterate over sub-rows
            real d_ij=d[j];
            for (int l=0;l<n1k;l++,ss+=sm,k+=km)
            {
                for (int m=0;m<n2k;m++)
                    ss[m] += d_ij * k[m];
            }
        }
    }
}

void convolve2Dbackprop(const Mat& source_image, const Mat& kernel,
                        const Mat& dC_ddest_image,
                        const Mat& dC_dsource_image, const Mat& dC_dkernel,
                        int step1, int step2, bool accumulate)
{
    int n1k=kernel.length();
    int n2k=kernel.width();
    int n1d=dC_ddest_image.length();
    int n2d=dC_ddest_image.width();
#ifdef BOUNDCHECK
    int n1s=source_image.length();
    int n2s=source_image.width();
    if (step1<1)
        PLERROR("convolve2Dbackprop: step1 (%d) should be a positive integer\n",
                step1);
    if (n1s!=step1*(n1d-1)+n1k)
        PLERROR("convolve2Dbackprop: source_image.length() (%d) should equal"
                " %d:\n"
                "step1 (%d) * (dest_image.length() (%d) - 1) + kernel.length()"
                " (%d)\n",
                n1s,step1*(n1d-1)+n1k,step1,n1d,n1k);
    if (dC_dsource_image.length()!=n1s)
        PLERROR("convolve2Dbackprop: source_image.length() (%d) should"
                " equal:\n"
                "dC_dsource_image.length() (%d)\n",
                n1s,dC_dsource_image.length());
    if (dC_dkernel.length()!=n1k)
        PLERROR("convolve2Dbackprop: kernel.length() (%d) should equal:\n"
                " dC_dkernel.length() (%d)\n",
                n1k,dC_dkernel.length());

    if (step2<1)
        PLERROR("convolve2Dbackprop: step2 (%d) should be a positive integer\n",
                step2);
    if (n2s!=step2*(n2d-1)+n2k)
        PLERROR("convolve2Dbackprop: source_image.width() (%d) should equal"
                " %d:\n"
                "step2 (%d) * (dest_image.width() (%d) - 1) + kernel.width()"
                " (%d)\n",
                n2s,step2*(n2d-1)+n2k,step2,n2d,n2k);
    if (dC_dsource_image.width()!=n2s)
        PLERROR("convolve2Dbackprop: source_image.width() (%d) should"
                " equal:\n"
                "dC_dsource_image.width() (%d)\n",
                n2s,dC_dsource_image.width());
    if (dC_dkernel.width()!=n2k)
        PLERROR("convolve2Dbackprop: kernel.width() (%d) should equal:\n"
                " dC_dkernel.width() (%d)\n",
                n2k,dC_dkernel.width());
#endif
    if (!accumulate)
    {
        dC_dsource_image.clear();
        dC_dkernel.clear();
    }
    int sm = source_image.mod();
    int dCdsm = dC_dsource_image.mod();
    int km = kernel.mod();
    int dCdkm = dC_dkernel.mod();
    int dCddm = dC_ddest_image.mod();

    real* s = source_image.data();
    real* dCds = dC_dsource_image.data();
    real* dCdd = dC_ddest_image.data();

    for (int i=0;i<n1d;i++,s+=sm*step1,dCds+=dCdsm*step1,dCdd+=dCddm)
    {
        real* s1 = s; // copy to iterate over columns
        real* dCds1 = dCds;
        for (int j=0;j<n2d;j++,s1+=step2,dCds1+=step2)
        {
            real* k = kernel.data();
            real* dCdk = dC_dkernel.data();
            real* ss = s1; // copy to iterate over sub-rows
            real* dCdss = dCds1;
            real dCdd_ij=dCdd[j];
            for (int l=0;l<n1k;l++,ss+=sm,dCdss+=dCdsm,k+=km,dCdk+=dCdkm)
            {
                for (int m=0;m<n2k;m++)
                {
                    dCdss[m] += dCdd_ij * k[m];
                    dCdk[m] += dCdd_ij * ss[m];
                }
            }
        }
    }
}


void convolve2Dbackprop(const Mat& source_image,
                        const Mat& dC_ddest_image,
                        const Mat& dC_dkernel,
                        int step1, int step2, bool accumulate)
{
    int n1k=dC_dkernel.length();
    int n2k=dC_dkernel.width();
    int n1d=dC_ddest_image.length();
    int n2d=dC_ddest_image.width();
#ifdef BOUNDCHECK
    int n1s=source_image.length();
    int n2s=source_image.width();
    if (step1<1)
        PLERROR("convolve2Dbackprop: step1 (%d) should be a positive integer\n",
                step1);
    if (n1s!=step1*(n1d-1)+n1k)
        PLERROR("convolve2Dbackprop: source_image.length() (%d) should equal"
                " %d:\n"
                "step1 (%d) * (dest_image.length() (%d) - 1) +"
                " dC_dkernel.length() (%d)\n",
                n1s,step1*(n1d-1)+n1k,step1,n1d,n1k);

    if (step2<1)
        PLERROR("convolve2Dbackprop: step2 (%d) should be a positive integer\n",
                step2);
    if (n2s!=step2*(n2d-1)+n2k)
        PLERROR("convolve2Dbackprop: source_image.width() (%d) should equal"
                " %d:\n"
                "step2 (%d) * (dest_image.width() (%d) - 1) +"
                " dC_dkernel.width() (%d)\n",
                n2s,step2*(n2d-1)+n2k,step2,n2d,n2k);
#endif
    if (!accumulate)
        dC_dkernel.clear();

    int sm = source_image.mod();
    int dCdkm = dC_dkernel.mod();
    int dCddm = dC_ddest_image.mod();

    real* s = source_image.data();
    real* dCdd = dC_ddest_image.data();

    for (int i=0;i<n1d;i++,s+=sm*step1,dCdd+=dCddm)
    {
        real* s1 = s; // copy to iterate over columns
        for (int j=0;j<n2d;j++,s1+=step2)
        {
            real* dCdk = dC_dkernel.data();
            real* ss = s1; // copy to iterate over sub-rows
            real dCdd_ij=dCdd[j];
            for (int l=0;l<n1k;l++,ss+=sm,dCdk+=dCdkm)
                for (int m=0;m<n2k;m++)
                    dCdk[m] += dCdd_ij * ss[m];
        }
    }
}


void backConvolve2Dbackprop(const Mat& kernel, const Mat& dest_image,
                            const Mat& dC_ddest_image,
                            const Mat& dC_dsource_image, const Mat& dC_dkernel,
                            int step1, int step2, bool accumulate)
{
    int n1k=kernel.length();
    int n2k=kernel.width();
    int n1d=dest_image.length();
    int n2d=dest_image.width();
#ifdef BOUNDCHECK
    int n1s=dC_dsource_image.length();
    int n2s=dC_dsource_image.width();
    if (step1<1)
        PLERROR("backConvolve2Dbackprop: step1 (%d) should be a positive"
                " integer\n",
                step1);
    if (n1s!=step1*(n1d-1)+n1k)
        PLERROR("backConvolve2Dbackprop: dC_dsource_image.length() (%d)\n"
                "should equal %d:\n"
                "step1 (%d) * (dest_image.length() (%d) - 1) + kernel.length()"
                " (%d)\n",
                n1s,step1*(n1d-1)+n1k,step1,n1d,n1k);
    if (dC_ddest_image.length()!=n1d)
        PLERROR("backConvolve2Dbackprop: dest_image.length() (%d) should"
                " equal:\n"
                "dC_ddest_image.length() (%d)\n",
                n1d,dC_ddest_image.length());
    if (dC_dkernel.length()!=n1k)
        PLERROR("backConvolve2Dbackprop: kernel.length() (%d) should equal:\n"
                " dC_dkernel.length() (%d)\n",
                n1k,dC_dkernel.length());

    if (step2<1)
        PLERROR("backConvolve2Dbackprop: step2 (%d) should be a positive"
                " integer\n",
                step2);
    if (n2s!=step2*(n2d-1)+n2k)
        PLERROR("backConvolve2Dbackprop: source_image.width() (%d)\n"
                "should equal %d:\n"
                "step2 (%d) * (dest_image.width() (%d) - 1) + kernel.width()"
                " (%d)\n",
                n2s,step2*(n2d-1)+n2k,step2,n2d,n2k);
    if (dC_ddest_image.width()!=n2d)
        PLERROR("backConvolve2Dbackprop: dest_image.width() (%d) should"
                " equal:\n"
                "dC_ddest_image.width() (%d)\n",
                n2d,dC_ddest_image.width());
    if (dC_dkernel.length()!=n2k)
        PLERROR("backConvolve2Dbackprop: kernel.width() (%d) should equal:\n"
                " dC_dkernel.width() (%d)\n",
                n2k,dC_dkernel.width());
#endif
    if (!accumulate)
    {
        dC_ddest_image.clear();
        dC_dkernel.clear();
    }
    int km = kernel.mod();
    int dCdkm = dC_dkernel.mod();
    int dm = dest_image.mod();
    int dCddm = dC_ddest_image.mod();
    int dCdsm = dC_dsource_image.mod();

    real* d = dest_image.data();
    real* dCdd = dC_ddest_image.data();
    real* dCds = dC_dsource_image.data();

    for (int i=0;i<n1d;i++,d+=dm,dCdd+=dCddm,dCds+=dCdsm*step1)
    {
        real* dCds1 = dCds; // copy to iterate over columns
        for (int j=0;j<n2d;j++,dCds1+=step2)
        {
            real* k = kernel.data();
            real* dCdk = dC_dkernel.data();
            real* dCdss = dCds1; // copy to iterate over sub-rows
            real d_ij=d[j];
            real dCdd_ij_sum = 0;
            for (int l=0;l<n1k;l++,dCdss+=dCdsm,k+=km,dCdk+=dCdkm)
            {
                for (int m=0;m<n2k;m++)
                {
                    dCdd_ij_sum += dCdss[m]*k[m];
                    dCdk[m] += dCdss[m]*d_ij;
                }
            }
            dCdd[j] += dCdd_ij_sum;
        }
    }
}

void backConvolve2Dbackprop(const Mat& dest_image,
                            const Mat& dC_dsource_image,
                            const Mat& dC_dkernel,
                            int step1, int step2, bool accumulate)
{
    int n1k=dC_dkernel.length();
    int n2k=dC_dkernel.width();
    int n1d=dest_image.length();
    int n2d=dest_image.width();
#ifdef BOUNDCHECK
    int n1s=dC_dsource_image.length();
    int n2s=dC_dsource_image.width();
    if (step1<1)
        PLERROR("backConvolve2Dbackprop: step1 (%d) should be a positive"
                " integer\n",
                step1);
    if (n1s!=step1*(n1d-1)+n1k)
        PLERROR("backConvolve2Dbackprop: dC_dsource_image.length() (%d)\n"
                "should equal %d:\n"
                "step1 (%d) * (dest_image.length() (%d) - 1) +"
                " dC_dkernel.length() (%d)\n",
                n1s,step1*(n1d-1)+n1k,step1,n1d,n1k);

    if (step2<1)
        PLERROR("backConvolve2Dbackprop: step2 (%d) should be a positive"
                " integer\n",
                step2);
    if (n2s!=step2*(n2d-1)+n2k)
        PLERROR("backConvolve2Dbackprop: source_image.width() (%d)\n"
                "should equal %d:\n"
                "step2 (%d) * (dest_image.width() (%d) - 1) +"
                " dC_dkernel.width() (%d)\n",
                n2s,step2*(n2d-1)+n2k,step2,n2d,n2k);
#endif
    if (!accumulate)
        dC_dkernel.clear();

    int dCdkm = dC_dkernel.mod();
    int dm = dest_image.mod();
    int dCdsm = dC_dsource_image.mod();

    real* d = dest_image.data();
    real* dCds = dC_dsource_image.data();

    for (int i=0;i<n1d;i++,d+=dm,dCds+=dCdsm*step1)
    {
        real* dCds1 = dCds; // copy to iterate over columns
        for (int j=0;j<n2d;j++,dCds1+=step2)
        {
            real* dCdk = dC_dkernel.data();
            real* dCdss = dCds1; // copy to iterate over sub-rows
            real d_ij=d[j];
            for (int l=0;l<n1k;l++,dCdss+=dCdsm,dCdk+=dCdkm)
                for (int m=0;m<n2k;m++)
                    dCdk[m] += dCdss[m]*d_ij;
        }
    }
}

} // end of namespace PLearn


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
