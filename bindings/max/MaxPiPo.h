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
#ifndef _MAX_PIPO_H_
#define _MAX_PIPO_H_

#include "PiPo.h"
#include "ext.h"
#include "ext_obex.h"
#include "ext_obex_util.h"

#ifdef PIPO_MAX_WITH_DOC
#include "ircammaxcapi.h"
#include <string.h>
#include <vector>
#endif // PIPO_MAX_WITH_DOC

typedef struct MaxPiPoSt {
  t_object head;
  PiPo *pipo;
  long verbose;
  
#ifdef PIPO_MAX_WITH_DOC
  void *dummy_attr[2];
  long dummy_attr_long;
  
  t_symbol classdigest;
  t_symbol classdescription;
  t_symbol classseealso;
  t_symbol externalversion;
#endif // PIPO_MAX_WITH_DOC
} MaxPiPoT;

#ifdef PIPO_MAX_WITH_DOC

#define atom_isnum(a) ((a)->a_type == A_LONG || (a)->a_type == A_FLOAT)
#define atom_issym(a) ((a)->a_type == A_SYM)
#define atom_isobj(p) ((p)->a_type == A_OBJ)
#define mysneg(s) ((s)->s_name)

#define atom_islong(p) ((p)->a_type == A_LONG)
#define atom_isfloat(p) ((p)->a_type == A_FLOAT)
#define atom_getnum(p) (((p)->a_type == A_FLOAT)? (double)((p)->a_w.w_float): (double)((p)->a_w.w_long))
#define atom_setvoid(p) ((p)->a_type = A_NOTHING)

#define PIPO_MAX_CLASS(pipoName, pipoClass, digest, description, numAttrs, attrNames, attrDescriptions) \
  PIPO_MAX_CLASS2(pipoName, pipoName, pipoClass, digest, description, numAttrs, attrNames, attrDescriptions)

// definition with long name for external, short name for attrs and help
// there is supposed to be an object alias in init/mubu-objectmappings.txt like this:
// max objectfile pipo.<short name> pipo.<long name>;
#define PIPO_MAX_CLASS2(pipoName, pipoShortName, pipoClass, digest, description, numAttrs, attrNames, attrDescriptions) \
  static t_class *max ## pipoClass ## Class = NULL; \
  static const unsigned int maxWordLen = 256; \
  static bool getPiPoInstanceAndAttrName(const char *attrName, char *instanceName, char *pipoAttrName){ \
    const char *dot = strrchr(attrName, '.');\
    if(dot != NULL){ \
      unsigned int pipoAttrNameLen = dot - attrName; \
      strcpy(pipoAttrName, dot + 1); \
      if(pipoAttrNameLen > maxWordLen) pipoAttrNameLen = maxWordLen; \
      strncpy(instanceName, attrName, pipoAttrNameLen); \
      instanceName[pipoAttrNameLen] = '\0';\
      return true;} \
    return false;}\
  static void getMaxAttributeList(PiPo *pipo, unsigned int attrId, long *pac, t_atom **pat){ \
    PiPo::Attr *attr = pipo->getAttr(attrId); \
    enum PiPo::Type type = attr->getType(); \
    switch(type){ \
      case PiPo::Undefined: \
        break; \
      case PiPo::Bool: \
      case PiPo::Enum: \
      case PiPo::Int: { \
        for(unsigned int i = 0; i < attr->getSize(); i++) atom_setlong((*pat) + i, attr->getInt(i)); \
        break; } \
      case PiPo::Float: \
      case PiPo::Double: { \
        for(unsigned int i = 0; i < attr->getSize(); i++) atom_setfloat((*pat) + i, attr->getDbl(i)); \
        break; } \
      case PiPo::String: \
      case PiPo::Dictionary: { \
        for(unsigned int i = 0; i < attr->getSize(); i++) atom_setsym((*pat) + i, gensym(attr->getStr(i))); \
        break; } \
      default: break;}} \
  static t_max_err setPiPoAttr(MaxPiPoT *self, void *attr, long ac, t_atom *at) { \
    const char *attrName = ((t_symbol *)object_method((t_object *)attr, gensym("getname")))->s_name;\
    char instanceName[maxWordLen]; \
    char pipoAttrName[maxWordLen]; \
    if(getPiPoInstanceAndAttrName(attrName, instanceName, pipoAttrName)) { \
      PiPo *pipo = self->pipo; \
      if(pipo != NULL) { \
        PiPo::Attr *attr = pipo->getAttr(pipoAttrName);\
        if(attr != NULL){ \
          for(int i = 0; i < ac; i++){ if(!atom_isnum(at + i) && !atom_issym(at + i)) ac = i; } \
        if(ac > 0) { \
          attr->setSize(ac); \
         if(atom_isnum(at) || atom_issym(at)){ \
           for(int i = 0; i < ac; i++) { \
             if(atom_issym(at + i)) { attr->set(i, (const char *)mysneg(atom_getsym(at + i)), true);} \
             else if(atom_islong(at + i)) { attr->set(i, (int)atom_getlong(at + i), true); } \
             else if(atom_isfloat(at + i)) { attr->set(i, (double)atom_getfloat(at + i), true);} \
             else { attr->set(i, 0, true);}} \
          attr->changed(true);}}}}} \
    return MAX_ERR_NONE; } \
  static t_max_err getPiPoAttr(MaxPiPoT *self, void *attr, long *pac, t_atom **pat) { \
    const char *attrName = ((t_symbol *)object_method((t_object *)attr, gensym("getname")))->s_name;\
    if(pac != NULL && pat != NULL){ \
      char instanceName[maxWordLen]; \
      char pipoAttrName[maxWordLen]; \
      *pac = 0; \
      if(getPiPoInstanceAndAttrName(attrName, instanceName, pipoAttrName)){ \
        PiPo *pipo = self->pipo; \
        if(pipo != NULL){ \
          PiPo::Attr *attr = pipo->getAttr(pipoAttrName);\
          if(attr != NULL){ \
            unsigned int attrSize = attr->getSize(); \
            char alloc; \
            if(atom_alloc_array(attrSize, pac, pat, &alloc) == MAX_ERR_NONE){ \
              getMaxAttributeList(pipo, attr->getIndex(), pac, pat); \
              *pac = attrSize; }}}}} \
    return MAX_ERR_NONE; } \
  static const char *getAttrDescription(const char *attrName){\
    for(int i = 0; i < numAttrs; i++){if((strcmp(attrName, attrNames[i])) == 0) return attrDescriptions[i];} \
    return ""; } \
  static void class_add_attributes_from_pipo(t_class *c) { \
    PiPo *pipo = new pipoClass(NULL); \
    unsigned int numAttributes = pipo->getNumAttrs(); \
    for(unsigned int iAttr = 0; iAttr < numAttributes; iAttr++){ \
      PiPo::Attr *attr = pipo->getAttr(iAttr); \
      std::string attrName = pipoShortName; \
      attrName += "."; \
      attrName += attr->getName(); \
      const char * attrName_c = attrName.c_str(); \
      const char * attrDescr = getAttrDescription(attrName_c); \
      enum PiPo::Type type = attr->getType(); \
      char attrIndexStr[256]; \
      snprintf(attrIndexStr, 256, "%d", 256 + iAttr); \
      switch(type) { \
        case PiPo::Undefined: \
          break; \
        case PiPo::Int: {\
          CLASS_ATTR_LONG(c, attrName_c, 0, MaxPiPoT, dummy_attr_long); \
          CLASS_ATTR_LABEL(c,attrName_c, 0, attrDescr); \
          CLASS_ATTR_ACCESSORS(c, attrName_c, getPiPoAttr, setPiPoAttr); \
          CLASS_ATTR_ORDER(c, attrName_c, 0, attrIndexStr); \
          IRCAMMAX_CLASS_ATTR_DESCRIPTION(c, attrName_c, attrDescr); \
          break; }\
        case PiPo::Bool: {\
          CLASS_ATTR_LONG(c, attrName_c, 0, MaxPiPoT, dummy_attr_long); \
          CLASS_ATTR_STYLE_LABEL(c, attrName_c, 0, "onoff", attrDescr); \
          CLASS_ATTR_ACCESSORS(c, attrName_c, getPiPoAttr, setPiPoAttr); \
          CLASS_ATTR_ORDER(c, attrName_c, 0, attrIndexStr); \
          IRCAMMAX_CLASS_ATTR_DESCRIPTION(c, attrName_c, attrDescr); \
          break; }\
        case PiPo::Enum: {\
          std::vector<const char *> *enumList = attr->getEnumList(); \
          std::string enumStr = (*enumList)[0]; \
          if(enumList != NULL && enumList->size() > 0){ \
            for(unsigned int i = 1; i < enumList->size(); i++){ \
              enumStr += " "; \
              enumStr += (*enumList)[i]; }} \
          CLASS_ATTR_LONG(c, attrName_c, 0, MaxPiPoT, dummy_attr_long); \
          CLASS_ATTR_LABEL(c,attrName_c, 0, attrDescr); \
          CLASS_ATTR_ACCESSORS(c, attrName_c, getPiPoAttr, setPiPoAttr); \
          CLASS_ATTR_ENUMINDEX(c, attrName_c, 0, enumStr.c_str());\
          CLASS_ATTR_ORDER(c, attrName_c, 0, attrIndexStr); \
          IRCAMMAX_CLASS_ATTR_DESCRIPTION(c, attrName_c, attrDescr); \
          break; }\
        case PiPo::Float: {\
          CLASS_ATTR_DOUBLE(c, attrName_c, 0, MaxPiPoT, dummy_attr); \
          CLASS_ATTR_LABEL(c,attrName_c, 0, attrDescr); \
          CLASS_ATTR_ACCESSORS(c, attrName_c, getPiPoAttr, setPiPoAttr); \
          CLASS_ATTR_ORDER(c, attrName_c, 0, attrIndexStr); \
          IRCAMMAX_CLASS_ATTR_DESCRIPTION(c, attrName_c, attrDescr); \
          break; }\
        case PiPo::Double: {\
          CLASS_ATTR_DOUBLE(c, attrName_c, 0, MaxPiPoT, dummy_attr); \
          CLASS_ATTR_LABEL(c,attrName_c, 0, attrDescr); \
          CLASS_ATTR_ACCESSORS(c, attrName_c, getPiPoAttr, setPiPoAttr); \
          CLASS_ATTR_ORDER(c, attrName_c, 0, attrIndexStr); \
          IRCAMMAX_CLASS_ATTR_DESCRIPTION(c, attrName_c, attrDescr); \
          break; }\
        case PiPo::String: \
        case PiPo::Dictionary: { \
          CLASS_ATTR_SYM(c, attrName_c, 0, MaxPiPoT, dummy_attr); \
          CLASS_ATTR_LABEL(c,attrName_c, 0, attrDescr); \
          CLASS_ATTR_ACCESSORS(c, attrName_c, getPiPoAttr, setPiPoAttr); \
          CLASS_ATTR_ORDER(c, attrName_c, 0, attrIndexStr); \
          IRCAMMAX_CLASS_ATTR_DESCRIPTION(c, attrName_c, attrDescr); \
          break; }\
        default: \
        break; }}}\
  void warn_deferred(MaxPiPoT *self);\
  static void *newMaxObject(t_symbol *s, long ac, t_atom *at) { \
    MaxPiPoT *self = (MaxPiPoT *)object_alloc(max ## pipoClass ## Class); \
    if(self != NULL) { \
      self->pipo = new pipoClass(NULL); \
    } \
    self->verbose = 1; \
    if(ac == 0) { \
      defer_low(self, (method) warn_deferred, NULL, 0, NULL);} \
    return self; } \
  void warn_deferred(MaxPiPoT *self) { \
    if(self->verbose != 0){ \
      object_error_obtrusive((t_object *)self, "pipo works only inside a pipo host!!! Double click or bang the object for more infos\n"); \
      object_warn((t_object *)self, "pipo works only inside a pipo host!!! Double click or bang the object for more infos\n"); } } \
  static void freeMaxObject(MaxPiPoT *self) { delete self->pipo; } \
  static void helpnameMethod(MaxPiPoT *self, char *str){ sprintf(str, "pipo.%s", pipoShortName);} \
  static void bangMethod(MaxPiPoT *self, t_symbol *s, short ac, t_atom *at){ stringload("HowToUsePiPoModules");} \
  static void listMethod(MaxPiPoT *self, t_symbol *s, short ac, t_atom *at){ \
    if(self->verbose != 0) object_error((t_object *) self, "pipo works only inside a pipo host!!! Double click or bang the object for more infos");}\
  static void intMethod(MaxPiPoT *self, long i){ \
    if(self->verbose != 0) object_error((t_object *) self, "pipo works only inside a pipo host!!! Double click or bang the object for more infos");} \
  static void floatMethod(MaxPiPoT *self, double f){ \
    if(self->verbose != 0) object_error((t_object *) self, "pipo works only inside a pipo host!!! Double click or bang the object for more infos");}\
  static void dblclickMethod(MaxPiPoT *self) { \
    stringload("HowToUsePiPoModules");} \
  void ext_main(void *r) { \
    t_class *c = class_new("pipo." pipoName, (method)newMaxObject, (method)freeMaxObject, (long)sizeof(MaxPiPoT), 0L, A_GIMME, 0); \
    class_initIrcamMax(c); \
    IRCAMMAX_CLASS_DIGEST(c, MaxPiPoT, digest); \
    IRCAMMAX_CLASS_DESCRIPTION(c, MaxPiPoT, description); \
    IRCAMMAX_CLASS_SEEALSO(c, MaxPiPoT, "pipo, pipo~, mubu.process"); \
    class_add_attributes_from_pipo(c); \
    class_addmethod(c, (method)helpnameMethod, "helpname", A_CANT, 0);\
    class_addmethod(c, (method)bangMethod, "bang", 0);\
    class_addmethod(c, (method)listMethod, "list", A_GIMME, 0);\
    class_addmethod(c, (method)intMethod, "int", A_LONG, 0);\
    class_addmethod(c, (method)floatMethod, "float", A_FLOAT, 0);\
    class_addmethod(c, (method)dblclickMethod,"dblclick", A_CANT, 0); \
    CLASS_ATTR_LONG(c, "verbose", 0, MaxPiPoT, verbose); \
    CLASS_ATTR_STYLE_LABEL(c, "verbose", 0, "onoff", "Print Errors and Warning"); \
    CLASS_ATTR_SAVE(c, "verbose", 0); \
    class_register(CLASS_BOX, c); \
    max ## pipoClass ## Class = c; }

#else // PIPO_MAX_WITH_DOC

#define PIPO_MAX_CLASS(pipoName, pipoClass) \
  PIPO_MAX_CLASS2(pipoName, pipoName, pipoClass)

// definition with long name for external, short name for attrs and help
// there is supposed to be an object alias in init/mubu-objectmappings.txt like this:
// max objectfile pipo.<short name> pipo.<long name>;
#define PIPO_MAX_CLASS2(pipoName, pipoShortName, pipoClass) \
  static t_class *max ## pipoClass ## Class = NULL; \
  static void *pipoPatchHandle; \
  void warn_deferred(MaxPiPoT *self);\
  static void *newMaxObject(t_symbol *s, long ac, t_atom *at) { \
   MaxPiPoT *self = (MaxPiPoT *)object_alloc(max ## pipoClass ## Class); \
   if(self != NULL) { self->pipo = new pipoClass(NULL); } \
   self->verbose = 1; \
   if(ac == 0) { \
     defer_low(self, (method) warn_deferred, NULL, 0, NULL);} \
  return self; } \
  void warn_deferred(MaxPiPoT *self) { \
    if(self->verbose != 0){ \
      object_error_obtrusive((t_object *)self, "pipo works only inside a pipo host!!! Double click or bang the object for more infos\n"); \
      object_warn((t_object *)self, "pipo works only inside a pipo host!!! Double click or bang the object for more infos\n"); } } \
  static void openPiPoPatch(){ \
   if(pipoPatchHandle != NULL){ \
     if(NOGOOD(pipoPatchHandle)) pipoPatchHandle= NULL; \
     else freeobject((t_object *)pipoPatchHandle);}\
   pipoPatchHandle = stringload("AboutPiPoModules"); } \
  static void freeMaxObject(MaxPiPoT *self) { delete self->pipo; } \
  static void helpnameMethod(MaxPiPoT *self, char *str){ sprintf(str, "pipo.%s", pipoShortName);} \
  static void bangMethod(MaxPiPoT *self, t_symbol *s, short ac, t_atom *at){ openPiPoPatch(); }\
  static void listMethod(MaxPiPoT *self, t_symbol *s, short ac, t_atom *at){ \
   if(self->verbose != 0) \
   object_error((t_object *) self, "pipo works only inside a pipo host!!! Double click or bang the object for more infos");} \
  static void intMethod(MaxPiPoT *self, long i){ if(self->verbose != 0) object_error((t_object *) self, "pipo works only inside a pipo host!!! Double click or bang the object for more infos");} \
  static void floatMethod(MaxPiPoT *self, double f){if(self->verbose != 0) object_error((t_object *) self, "pipo works only inside a pipo host!!! Double click or bang the object for more infos");}\
  static void dblclickMethod(MaxPiPoT *self) { \
    openPiPoPatch();} \
  void ext_main(void *r) { \
    t_class *c = class_new("pipo." pipoName, (method)newMaxObject, (method)freeMaxObject, (long)sizeof(MaxPiPoT), 0L, A_GIMME, 0); \
    class_addmethod(c, (method)helpnameMethod, "helpname", A_CANT, 0);\
    class_addmethod(c, (method)bangMethod, "bang", 0); \
    class_addmethod(c, (method)listMethod, "list", A_GIMME, 0); \
    class_addmethod(c, (method)intMethod, "int", A_LONG, 0); \
    class_addmethod(c, (method)floatMethod, "float", A_FLOAT, 0); \
    class_addmethod(c, (method)dblclickMethod,"dblclick", A_CANT, 0); \
    CLASS_ATTR_LONG(c, "verbose", 0, MaxPiPoT, verbose); \
    CLASS_ATTR_STYLE_LABEL(c, "verbose", 0, "onoff", "Print Errors and Warning"); \
    CLASS_ATTR_SAVE(c, "verbose", 0); \
    class_register(CLASS_BOX, c); \
    max ## pipoClass ## Class = c; }

#endif // PIPO_MAX_WITH_DOC

#endif // _MAX_PIPO_H_
