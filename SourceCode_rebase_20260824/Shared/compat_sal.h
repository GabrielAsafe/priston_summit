#ifndef SHARED_COMPAT_SAL_H
#define SHARED_COMPAT_SAL_H

/*
 * Compatibilidade das anotações SAL do Microsoft DirectX SDK
 * para compilação com MinGW.
 *
 * Estas macros são apenas anotações de análise estática no MSVC.
 * Não alteram ABI, layout de estruturas ou chamadas de função.
 */

#ifndef __in
#define __in
#endif

#ifndef __in_opt
#define __in_opt
#endif

#ifndef __out
#define __out
#endif

#ifndef __out_opt
#define __out_opt
#endif

#ifndef __inout
#define __inout
#endif

#ifndef __inout_opt
#define __inout_opt
#endif

#ifndef __deref_in
#define __deref_in
#endif

#ifndef __deref_in_opt
#define __deref_in_opt
#endif

#ifndef __deref_out
#define __deref_out
#endif

#ifndef __deref_out_opt
#define __deref_out_opt
#endif

#ifndef __deref_inout
#define __deref_inout
#endif

#ifndef __deref_inout_opt
#define __deref_inout_opt
#endif

#ifndef __reserved
#define __reserved
#endif

#ifndef __success
#define __success(x)
#endif

#ifndef __checkReturn
#define __checkReturn
#endif

#ifndef __mustCheck
#define __mustCheck
#endif

#ifndef __nullterminated
#define __nullterminated
#endif

#ifndef __nullnullterminated
#define __nullnullterminated
#endif

#ifndef __possibly_notnullterminated
#define __possibly_notnullterminated
#endif

#ifndef __format_string
#define __format_string
#endif

#ifndef __blocksOn
#define __blocksOn(x)
#endif

#ifndef __fallthrough
#define __fallthrough
#endif

#ifndef __analysis_assume
#define __analysis_assume(x)
#endif

#ifndef __in_bcount
#define __in_bcount(x)
#endif

#ifndef __in_bcount_opt
#define __in_bcount_opt(x)
#endif

#ifndef __out_bcount
#define __out_bcount(x)
#endif

#ifndef __out_bcount_opt
#define __out_bcount_opt(x)
#endif

#ifndef __inout_bcount
#define __inout_bcount(x)
#endif

#ifndef __inout_bcount_opt
#define __inout_bcount_opt(x)
#endif

#ifndef __in_ecount
#define __in_ecount(x)
#endif

#ifndef __in_ecount_opt
#define __in_ecount_opt(x)
#endif

#ifndef __out_ecount
#define __out_ecount(x)
#endif

#ifndef __out_ecount_opt
#define __out_ecount_opt(x)
#endif

#ifndef __inout_ecount
#define __inout_ecount(x)
#endif

#ifndef __inout_ecount_opt
#define __inout_ecount_opt(x)
#endif

#ifndef __deref_in_bcount
#define __deref_in_bcount(x)
#endif

#ifndef __deref_in_bcount_opt
#define __deref_in_bcount_opt(x)
#endif

#ifndef __deref_out_bcount
#define __deref_out_bcount(x)
#endif

#ifndef __deref_out_bcount_opt
#define __deref_out_bcount_opt(x)
#endif

#ifndef __deref_inout_bcount
#define __deref_inout_bcount(x)
#endif

#ifndef __deref_inout_bcount_opt
#define __deref_inout_bcount_opt(x)
#endif

#ifndef __deref_in_ecount
#define __deref_in_ecount(x)
#endif

#ifndef __deref_in_ecount_opt
#define __deref_in_ecount_opt(x)
#endif

#ifndef __deref_out_ecount
#define __deref_out_ecount(x)
#endif

#ifndef __deref_out_ecount_opt
#define __deref_out_ecount_opt(x)
#endif

#ifndef __deref_inout_ecount
#define __deref_inout_ecount(x)
#endif

#ifndef __deref_inout_ecount_opt
#define __deref_inout_ecount_opt(x)
#endif

#ifndef __out_bcount_part
#define __out_bcount_part(size, length)
#endif

#ifndef __out_bcount_full
#define __out_bcount_full(size)
#endif

#ifndef __out_ecount_part
#define __out_ecount_part(size, length)
#endif

#ifndef __out_ecount_full
#define __out_ecount_full(size)
#endif

#ifndef __inexpressible_readableTo
#define __inexpressible_readableTo(x)
#endif

#ifndef __inexpressible_writableTo
#define __inexpressible_writableTo(x)
#endif

/*
 * Algumas versões do DirectX SDK usam nomes SAL__ internos.
 */

#ifndef SAL__in
#define SAL__in
#endif

#ifndef SAL__in_opt
#define SAL__in_opt
#endif

#ifndef SAL__out
#define SAL__out
#endif

#ifndef SAL__out_opt
#define SAL__out_opt
#endif

#ifndef SAL__in_bcount
#define SAL__in_bcount(x)
#endif

#ifndef SAL__in_bcount_opt
#define SAL__in_bcount_opt(x)
#endif

#ifndef SAL__out_bcount
#define SAL__out_bcount(x)
#endif

#ifndef SAL__out_bcount_opt
#define SAL__out_bcount_opt(x)
#endif

#ifndef SAL__in_ecount
#define SAL__in_ecount(x)
#endif

#ifndef SAL__in_ecount_opt
#define SAL__in_ecount_opt(x)
#endif

#ifndef SAL__out_ecount
#define SAL__out_ecount(x)
#endif

#ifndef SAL__out_ecount_opt
#define SAL__out_ecount_opt(x)
#endif

#ifndef SAL__deref_out_bcount
#define SAL__deref_out_bcount(x)
#endif

#ifndef SAL__deref_out_bcount_opt
#define SAL__deref_out_bcount_opt(x)
#endif

#ifndef SAL__deref_out_ecount
#define SAL__deref_out_ecount(x)
#endif

#ifndef SAL__deref_out_ecount_opt
#define SAL__deref_out_ecount_opt(x)
#endif

#endif
