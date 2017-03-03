/**
 *
 * @file MaxPiPo.h
 * @author Norbert.Schnell@ircam.fr
 * 
 * @brief Max/MSP extension of Plugin Interface for Processing Objects
 * 
 * Copyright (C) 2012-2014 by IRCAM – Centre Pompidou, Paris, France.
 * All rights reserved.
 * 
 */
#ifndef _MAX_PIPO_
#define _MAX_PIPO_

#include "PiPo.h"
#include "ext.h"
#include "ext_obex.h"

typedef struct MaxPiPoSt {
  t_object head;
  PiPo *pipo;
} MaxPiPoT;

#define PIPO_MAX_CLASS(pipoName, pipoClass) \
  static t_class *max ## pipoClass ## Class = NULL; \
  static void *newMaxObject(t_symbol *s, long ac, t_atom *at) { \
    MaxPiPoT *self = (MaxPiPoT *)object_alloc(max ## pipoClass ## Class); \
    if(self != NULL) { self->pipo = new pipoClass(NULL); } \
    return self; } \
  static void freeMaxObject(MaxPiPoT *self) { delete self->pipo; } \
  static void helpnameMethod(MaxPiPoT *self, char *str){ sprintf(str, "pipo.%s", pipoName);} \
  static void bangMethod(MaxPiPoT *self, t_symbol *s, short ac, t_atom *at){object_error((t_object *) self, "pipo works only inside a pipo host!!!");} \
  static void listMethod(MaxPiPoT *self, t_symbol *s, short ac, t_atom *at){object_error((t_object *) self, "pipo works only inside a pipo host!!!");}\
  static void intMethod(MaxPiPoT *self, long i){object_error((t_object *) self, "pipo works only inside a pipo host!!!");} \
  static void floatMethod(MaxPiPoT *self, double f){object_error((t_object *) self, "pipo works only inside a pipo host!!!");}\
  int main(void) { \
    t_class *c = class_new("pipo." pipoName, (method)newMaxObject, (method)freeMaxObject, (long)sizeof(MaxPiPoT), 0L, A_GIMME, 0); \
    class_addmethod(c, (method)helpnameMethod, "helpname", A_CANT, 0);\
    class_addmethod(c, (method)bangMethod, "bang", 0);\
    class_addmethod(c, (method)listMethod, "list", A_GIMME, 0);\
    class_addmethod(c, (method)intMethod, "int", A_LONG, 0);\
    class_addmethod(c, (method)floatMethod, "float", A_FLOAT, 0);\
    class_register(CLASS_BOX, c); \
    max ## pipoClass ## Class = c; \
    return 0; }

#endif
