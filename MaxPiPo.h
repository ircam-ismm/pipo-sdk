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
  int main(void) { \
    t_class *c = class_new("pipo." pipoName, (method)newMaxObject, (method)freeMaxObject, (long)sizeof(MaxPiPoT), 0L, A_GIMME, 0); \
    class_register(CLASS_NOBOX, c); \
    max ## pipoClass ## Class = c; \
    return 0; }

#endif
