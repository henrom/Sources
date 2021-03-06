/*******************************************************************
 *  File:    omalloc.h
 *  Purpose: declaration of public routines for omalloc
 *  Author:  obachman@mathematik.uni-kl.de (Olaf Bachmann)
 *  Created: 11/99
 *******************************************************************/
#ifndef OM_ALLOC_H
#define OM_ALLOC_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <omalloc/omConfig.h>

#if defined(OM_NDEBUG) && !defined(OM_ALLOC_INTERNAL)
#if (SIZEOF_LONG == 8)
#define OM_T_FREE1
#define OM_T_FREE3
#define OM_T_STR
#define OM_T_ALLOC
#define OM_T_REALLOC
#endif
#endif

#include <omalloc/omDerivedConfig.h>
#include <omalloc/omError.h>
#include <omalloc/omStructs.h>
#include <omalloc/omAllocDecl.h>
#include <omalloc/omInlineDecl.h>
#include <omalloc/omBin.h>
#include <omalloc/omMemOps.h>
#include <omalloc/omList.h>
#include <omalloc/omGetBackTrace.h>
#include <omalloc/omRet2Info.h>
#include <omalloc/omStats.h>
#include <omalloc/omOpts.h>
#include <omalloc/omBinPage.h>
#include <omalloc/omAllocSystem.h>
#include <omalloc/omTables.h>
#include <omalloc/omAllocPrivate.h>
#include <omalloc/omDebug.h>
#include <omalloc/omInline.h>
#include <omalloc/omAllocFunc.h>

#ifdef __cplusplus
}
#endif

#endif /* OM_ALLOC_H */
