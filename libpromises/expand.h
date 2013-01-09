/*

   Copyright (C) Cfengine AS

   This file is part of Cfengine 3 - written and maintained by Cfengine AS.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; version 3.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA

  To the extent this program is licensed as part of the Enterprise
  versions of Cfengine, the applicable Commerical Open Source License
  (COSL) may apply to this file if you as a licensee so wish it. See
  included file COSL.txt.
*/

#ifndef CFENGINE_EXPAND_H
#define CFENGINE_EXPAND_H

#include "cf3.defs.h"

#include "reporting.h"

/**
  @brief Promise expansion and execution.

  This file contains functions to expand promises and execute them.
  Some functions in this file assume that there are no NULL values, i.e. the caller
  checks that before calling the callee.
*/

int ExpandPromise(AgentType ag, const char *scopeid, Promise *pp, void *fnptr, const ReportContext *report_context);
void ExpandPromiseAndDo(AgentType ag, const char *scope, Promise *p, Rlist *scalarvars, Rlist *listvars,
                        void (*fnptr) (), const ReportContext *report_context);
Rval ExpandDanglers(const char *scope, Rval rval, const Promise *pp);
void MapIteratorsFromRval(const char *scope, Rlist **los, Rlist **lol, Rval rval, const Promise *pp);

bool IsExpandable(const char *str);
int ExpandScalar(const char *string, char buffer[CF_EXPANDSIZE]);
Rval ExpandBundleReference(const char *scopeid, Rval rval);
FnCall *ExpandFnCall(const char *contextid, FnCall *f, int expandnaked);
Rval ExpandPrivateRval(const char *contextid, Rval rval);
Rlist *ExpandList(const char *scopeid, const Rlist *list, int expandnaked);
Rval EvaluateFinalRval(const char *scopeid, Rval rval, int forcelist, const Promise *pp);

/**
  @brief Checks if the variable name is legal or not.

  This check takes a string and verifies that only legal symbols have been used. It detects
  both naked and non-naked variables. Valid variable names contain only the following characters
  'a-zA-Z0-9_' and in the case of non-naked variables '@()'.
  @param [in] str The string containing the variable name to examine
  @return True if the name is valid, false in any other case.
  */
bool IsLegalVariableName(const char *str);
/**
  @brief Detects if a variable is naked or not.

  Notice that this does not mean that it detects if a variable is correctly formed or not, only if it
  is naked or not.
  @param [in] str The string containing the variable name to examine.
  @return True if it is a naked variable, false if not.
  */
bool IsNakedVar(const char *str);

/**
  @brief Eliminates decoration from a variable.

  Eliminating decorations from a variable means removing the extra characters at the beginning and at
  the end. This function assumes that all variables are valid and that no NULL pointers are passed to
  it. It is also important to take into account that this function calls IsNakedVar, since it needs to
  check if a variable is naked before attempting to remove the decorations from it.
  @param [out] s1 Destination char * where the naked variable will be stored.
  @param [in] s2 Source char * containing the variable with its decorations.
  @return Returns 0 if it was possible to remove the decorations from the variable and -1 if it was not possible.
  */
int GetNaked(char *s1, const char *s2);
void ConvergeVarHashPromise(char *scope, const Promise *pp, int checkdup);
int ExpandPrivateScalar(const char *contextid, const char *string, char buffer[CF_EXPANDSIZE]);

#endif
