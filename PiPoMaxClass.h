/**
 *
 * @file PiPoMaxClass.h
 * @author Norbert.Schnell@ircam.fr
 * 
 * @brief Max/MSP extension of Plugin Interface for Processing Objects
 * 
 * Copyright (C) 2012 by IMTR IRCAM – Centre Pompidou, Paris, France.
 * All rights reserved.
 * 
 */
#include "ext.h"
#include "ext_obex.h"

#ifndef mysneg
#define atom_isnum(a) ((a)->a_type == A_LONG || (a)->a_type == A_FLOAT)
#define atom_issym(a) ((a)->a_type == A_SYM)
#define atom_isobj(p) ((p)->a_type == A_OBJ) 
#define mysneg(s) ((s)->s_name)
#endif

#ifndef PIPO_MAX_CLASS

#include <new>

#define PIPO_MAX_CLASS(pipoClassName, pipoClass) \
  typedef struct Max ## pipoClass ## St { t_object o; pipoClass pipoClassName; } Max ## pipoClass ## T; \
  static t_class *pipoClassName = NULL; \

#define PIPO_MAX_MAIN(pipoClassName, pipoClass, pipoName) \
  static void *newMaxObject(t_symbol *s, long ac, t_atom *at) { \
    Max ## pipoClass ## T *self = (Max ## pipoClass ## T *)object_alloc(pipoClassName); \
    if(self != NULL) new(&self->pipoClassName) pipoClass(); \
    return self; } \
  static void freeMaxObject(Max ## pipoClass ## T *self) { self->pipoClassName.~pipoClass(); } \
  int main(void) { \
    t_class *c = class_new("pipo." pipoName, (method)newMaxObject, (method)freeMaxObject, (long)sizeof(Max ## pipoClass ## T), 0L, A_GIMME, 0); \
    class_register(CLASS_NOBOX, c); \
    pipoClassName = c;

#define PIPO_MAX_EXIT() } return 0

#define PIPO_MAX_ATTR_SETTER_LONG(pipoClassName, pipoClass, maxMeth, pipoMeth) \
  static t_max_err maxMeth(t_object *o, void *attr, long n, t_atom *a) { \
    Max ## pipoClass ## T *self = (Max ## pipoClass ## T *)o; \
    if(n > 0 && atom_isnum(a)) self->pipoClassName.pipoMeth(atom_getlong(a)); \
    return MAX_ERR_NONE; }

#define PIPO_MAX_ATTR_SETTER_FLOAT(pipoClassName, pipoClass, maxMeth, pipoMeth) \
  static t_max_err maxMeth(t_object *o, void *attr, long n, t_atom *a) { \
    Max ## pipoClass ## T *self = (Max ## pipoClass ## T *)o; \
    if(n > 0 && atom_isnum(a)) self->pipoClassName.pipoMeth(atom_getfloat(a)); \
    return MAX_ERR_NONE; }

#define PIPO_MAX_ATTR_SETTER_ENUM(pipoClassName, pipoClass, maxMeth, pipoMeth) \
static t_max_err maxMeth(t_object *o, void *attr, long n, t_atom *a) { \
  Max ## pipoClass ## T *self = (Max ## pipoClass ## T *)o; \
  t_symbol *attrName = (t_symbol *)object_method((t_object *)attr, gensym("getname")); \
  if(n > 0) { \
    if(atom_isnum(a)) self->pipoClassName.pipoMeth(atom_getlong(a)); \
    else if(atom_issym(a)) { \
      t_symbol *sym = atom_getsym(a); \
      if(!self->pipoClassName.pipoMeth(mysneg(sym))) object_error((t_object *)self, "invalid argument '%s' for %s", mysneg(sym), mysneg(attrName)); } \
    else object_error((t_object *)self, "invalid argument for %s", mysneg(attrName)); } \
  else object_error((t_object *)self, "missing argument for %s", mysneg(attrName)); \
  return MAX_ERR_NONE; }

#endif
