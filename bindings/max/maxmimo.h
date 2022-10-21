/**
 *
 * @file max_mimo.h
 * @author Diemo Schwarz
 * 
 * @brief Max/MSP wrapper generator for a mimo module
 * 
 * Copyright (C) 2016 by IRCAM – Centre Pompidou, Paris, France.
 * All rights reserved.
 * 
 */
#ifndef _MAX_MIMO_
#define _MAX_MIMO_

#include "mimo.h"
#include "ext.h"
#include "ext_obex.h"
#include "ext_obex_util.h"

#ifdef PIPO_MAX_WITH_DOC
#include "ircammaxcapi.h"
#include <string.h>
#include <vector>
#endif // PIPO_MAX_WITH_DOC

typedef struct max_mimo_st {
  t_object head;
  Mimo *mimo;
  long verbose;
  
#ifdef PIPO_MAX_WITH_DOC
  void *dummy_attr[2];
  long dummy_attr_long;

  t_symbol classdigest;
  t_symbol classdescription;
  t_symbol classseealso;
  t_symbol externalversion;
#endif // PIPO_MAX_WITH_DOC
} MaxMimoT;

#ifdef PIPO_MAX_WITH_DOC

#define atom_isnum(a) ((a)->a_type == A_LONG || (a)->a_type == A_FLOAT)
#define atom_issym(a) ((a)->a_type == A_SYM)
#define atom_isobj(p) ((p)->a_type == A_OBJ)
#define mysneg(s) ((s)->s_name)

#define atom_islong(p) ((p)->a_type == A_LONG)
#define atom_isfloat(p) ((p)->a_type == A_FLOAT)
#define atom_getnum(p) (((p)->a_type == A_FLOAT)? (double)((p)->a_w.w_float): (double)((p)->a_w.w_long))
#define atom_setvoid(p) ((p)->a_type = A_NOTHING)

#define MIMO_MAX_CLASS(mimoName, mimoClass, digest, description, numAttrs, attrNames, attrDescriptions) \
  MIMO_MAX_CLASS2(mimoName, mimoName, mimoClass, digest, description, numAttrs, attrNames, attrDescriptions)

// definition with long name for external, short name for attrs and help
// there is supposed to be an object alias in init/mubu-objectmappings.txt like this:
// max objectfile mimo.<short name> mimo.<long name>;
#define MIMO_MAX_CLASS2(mimoName, mimoShortName, mimoClass, digest, description, numAttrs, attrNames, attrDescriptions) \
  static t_class *max ## mimoClass ## Class = NULL; \
  static const unsigned int maxWordLen = 256; \
  static bool getMiMoInstanceAndAttrName(const char *attrName, char *instanceName, char *mimoAttrName){ \
    const char *dot = strrchr(attrName, '.');\
    if(dot != NULL){ \
      unsigned int mimoAttrNameLen = dot - attrName; \
      strcpy(mimoAttrName, dot + 1); \
      if(mimoAttrNameLen > maxWordLen) mimoAttrNameLen = maxWordLen; \
      strncpy(instanceName, attrName, mimoAttrNameLen); \
      instanceName[mimoAttrNameLen] = '\0';\
      return true;} \
    return false;}\
  static void getMaxAttributeList(Mimo *mimo, unsigned int attrId, long *pac, t_atom **pat){ \
    PiPo::Attr *attr = mimo->getAttr(attrId); \
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
  static t_max_err setMiMoAttr(MaxMimoT *self, void *attr, long ac, t_atom *at) { \
    const char *attrName = ((t_symbol *)object_method((t_object *)attr, gensym("getname")))->s_name;\
    char instanceName[maxWordLen]; \
    char mimoAttrName[maxWordLen]; \
    if(getMiMoInstanceAndAttrName(attrName, instanceName, mimoAttrName)) { \
      Mimo *mimo = self->mimo; \
      if(mimo != NULL) { \
        PiPo::Attr *attr = mimo->getAttr(mimoAttrName);\
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
  static t_max_err getMiMoAttr(MaxMimoT *self, void *attr, long *pac, t_atom **pat) { \
    const char *attrName = ((t_symbol *)object_method((t_object *)attr, gensym("getname")))->s_name;\
    if(pac != NULL && pat != NULL){ \
      char instanceName[maxWordLen]; \
      char mimoAttrName[maxWordLen]; \
      *pac = 0; \
      if(getMiMoInstanceAndAttrName(attrName, instanceName, mimoAttrName)){ \
        Mimo *mimo = self->mimo; \
        if(mimo != NULL){ \
          PiPo::Attr *attr = mimo->getAttr(mimoAttrName);\
          if(attr != NULL){ \
            unsigned int attrSize = attr->getSize(); \
            char alloc; \
            if(atom_alloc_array(attrSize, pac, pat, &alloc) == MAX_ERR_NONE){ \
              getMaxAttributeList(mimo, attr->getIndex(), pac, pat); \
              *pac = attrSize; }}}}} \
    return MAX_ERR_NONE; } \
  static const char *getAttrDescription(const char *attrName){\
    for(int i = 0; i < numAttrs; i++){if((strcmp(attrName, attrNames[i])) == 0) return attrDescriptions[i];} \
    return ""; } \
  static void class_add_attributes_from_mimo(t_class *c) { \
    Mimo *mimo = new mimoClass(NULL); \
    unsigned int numAttributes = mimo->getNumAttrs(); \
    for(unsigned int iAttr = 0; iAttr < numAttributes; iAttr++){ \
      PiPo::Attr *attr = mimo->getAttr(iAttr); \
      std::string attrName = mimoShortName; \
      attrName += "."; \
      attrName += attr->getName(); \
      const char * attrName_c = attrName.c_str(); \
      const char * attrDescr = getAttrDescription(attrName_c); \
      enum PiPo::Type type = attr->getType(); \
      char attrIndexStr[256]; \
      sprintf(attrIndexStr, "%d", 256 + iAttr); \
      switch(type) { \
        case PiPo::Undefined: \
          break; \
        case PiPo::Int: {\
          CLASS_ATTR_LONG(c, attrName_c, 0, MaxMimoT, dummy_attr_long); \
          CLASS_ATTR_LABEL(c,attrName_c, 0, attrDescr); \
          CLASS_ATTR_ACCESSORS(c, attrName_c, getMiMoAttr, setMiMoAttr); \
          CLASS_ATTR_ORDER(c, attrName_c, 0, attrIndexStr); \
          IRCAMMAX_CLASS_ATTR_DESCRIPTION(c, attrName_c, attrDescr); \
          break; }\
        case PiPo::Bool: {\
          CLASS_ATTR_LONG(c, attrName_c, 0, MaxMimoT, dummy_attr_long); \
          CLASS_ATTR_STYLE_LABEL(c, attrName_c, 0, "onoff", attrDescr); \
          CLASS_ATTR_ACCESSORS(c, attrName_c, getMiMoAttr, setMiMoAttr); \
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
          CLASS_ATTR_LONG(c, attrName_c, 0, MaxMimoT, dummy_attr_long); \
          CLASS_ATTR_LABEL(c,attrName_c, 0, attrDescr); \
          CLASS_ATTR_ACCESSORS(c, attrName_c, getMiMoAttr, setMiMoAttr); \
          CLASS_ATTR_ENUMINDEX(c, attrName_c, 0, enumStr.c_str());\
          CLASS_ATTR_ORDER(c, attrName_c, 0, attrIndexStr); \
          IRCAMMAX_CLASS_ATTR_DESCRIPTION(c, attrName_c, attrDescr); \
          break; }\
        case PiPo::Float: {\
          CLASS_ATTR_DOUBLE(c, attrName_c, 0, MaxMimoT, dummy_attr); \
          CLASS_ATTR_LABEL(c,attrName_c, 0, attrDescr); \
          CLASS_ATTR_ACCESSORS(c, attrName_c, getMiMoAttr, setMiMoAttr); \
          CLASS_ATTR_ORDER(c, attrName_c, 0, attrIndexStr); \
          IRCAMMAX_CLASS_ATTR_DESCRIPTION(c, attrName_c, attrDescr); \
          break; }\
        case PiPo::Double: {\
          CLASS_ATTR_DOUBLE(c, attrName_c, 0, MaxMimoT, dummy_attr); \
          CLASS_ATTR_LABEL(c,attrName_c, 0, attrDescr); \
          CLASS_ATTR_ACCESSORS(c, attrName_c, getMiMoAttr, setMiMoAttr); \
          CLASS_ATTR_ORDER(c, attrName_c, 0, attrIndexStr); \
          IRCAMMAX_CLASS_ATTR_DESCRIPTION(c, attrName_c, attrDescr); \
          break; }\
        case PiPo::String: \
        case PiPo::Dictionary: { \
          CLASS_ATTR_SYM(c, attrName_c, 0, MaxMimoT, dummy_attr); \
          CLASS_ATTR_LABEL(c,attrName_c, 0, attrDescr); \
          CLASS_ATTR_ACCESSORS(c, attrName_c, getMiMoAttr, setMiMoAttr); \
          CLASS_ATTR_ORDER(c, attrName_c, 0, attrIndexStr); \
          IRCAMMAX_CLASS_ATTR_DESCRIPTION(c, attrName_c, attrDescr); \
          break; }\
        default: \
        break; }}}\
  void warn_deferred(MaxMimoT *self);\
  static void *newMaxObject(t_symbol *s, long ac, t_atom *at) { \
    MaxMimoT *self = (MaxMimoT *)object_alloc(max ## mimoClass ## Class); \
    if(self != NULL) self->mimo = new mimoClass(NULL); \
    self->verbose = 1; \
    if(ac == 0) defer_low(self, (method) warn_deferred, NULL, 0, NULL); \
    return self; } \
  void warn_deferred(MaxMimoT *self) { \
    if(self->verbose != 0){ \
      object_error_obtrusive((t_object *)self, "mimo works only inside a mimo host!!! see mubu.model and pipo\n"); \
      object_warn((t_object *)self, "mimo works only inside a mimo host!!! see mubu.model and pipo\n"); } } \
  static void freeMaxObject(MaxMimoT *self) { delete self->mimo; } \
  static void helpnameMethod(MaxMimoT *self, char *str){ sprintf(str, "mimo.%s", mimoShortName);} \
  static void bangMethod(MaxMimoT *self, t_symbol *s, short ac, t_atom *at){ \
    if(self->verbose != 0) object_error((t_object *) self, "mimo works only inside a mimo host!!! see mubu.model and pipo");} \
  static void listMethod(MaxMimoT *self, t_symbol *s, short ac, t_atom *at){ \
    if(self->verbose != 0) object_error((t_object *) self, "mimo works only inside a mimo host!!! see mubu.model and pipo");}\
  static void intMethod(MaxMimoT *self, long i){ \
    if(self->verbose != 0) object_error((t_object *) self, "mimo works only inside a mimo host!!! see mubu.model and pipo");} \
  static void floatMethod(MaxMimoT *self, double f){ \
    if(self->verbose != 0) object_error((t_object *) self, "mimo works only inside a mimo host!!! see mubu.model and pipo");}\
  void ext_main(void *r) { \
    t_class *c = class_new("mimo." mimoName, (method)newMaxObject, (method)freeMaxObject, (long)sizeof(MaxMimoT), 0L, A_GIMME, 0); \
    class_initIrcamMax(c); \
    IRCAMMAX_CLASS_DIGEST(c, MaxMimoT, digest); \
    IRCAMMAX_CLASS_DESCRIPTION(c, MaxMimoT, description); \
    IRCAMMAX_CLASS_SEEALSO(c, MaxMimoT, "mubu.model"); \
    class_add_attributes_from_mimo(c); \
    class_addmethod(c, (method)helpnameMethod, "helpname", A_CANT, 0);\
    class_addmethod(c, (method)bangMethod, "bang", 0);\
    class_addmethod(c, (method)listMethod, "list", A_GIMME, 0);\
    class_addmethod(c, (method)intMethod, "int", A_LONG, 0);\
    class_addmethod(c, (method)floatMethod, "float", A_FLOAT, 0);\
    CLASS_ATTR_LONG(c, "verbose", 0, MaxMimoT, verbose); \
    CLASS_ATTR_STYLE_LABEL(c, "verbose", 0, "onoff", "Print Errors and Warning"); \
    CLASS_ATTR_SAVE(c, "verbose", 0); \
    class_register(CLASS_BOX, c); \
    max ## mimoClass ## Class = c; }

#else // PIPO_MAX_WITH_DOC

#define MIMO_MAX_CLASS(mimoName, mimoClass) \
  MIMO_MAX_CLASS2(mimoName, mimoName, mimoClass)

// definition with long name for external, short name for attrs and help
// there is supposed to be an object alias in init/mubu-objectmappings.txt like this:
// max objectfile mimo.<short name> mimo.<long name>;
#define MIMO_MAX_CLASS2(mimoName, mimoShortName, mimoClass) \
  static t_class *max_ ## mimoClass ## _class = NULL; \
  void warn_deferred(MaxMimoT *self);\
  static void *new_max_object(t_symbol *s, long ac, t_atom *at) { \
    MaxMimoT *self = (MaxMimoT *) object_alloc(max_ ## mimoClass ## _class); \
    if (self != NULL) { self->mimo = new mimoClass(NULL); } \
  self->verbose = 1; \
  if(ac == 0) defer_low(self, (method) warn_deferred, NULL, 0, NULL); \
  return self; } \
  void warn_deferred(MaxMimoT *self) { \
  if(self->verbose != 0){ \
    object_error_obtrusive((t_object *)self, "mimo works only inside a mimo host!!! see mubu.model and pipo\n"); \
    object_warn((t_object *)self, "mimo works only inside a mimo host!!! see mubu.model and pipo\n"); } } \
  static void free_max_object(MaxMimoT *self) { delete self->mimo; } \
  static void helpnameMethod(MaxMimoT *self, char *str){ sprintf(str, "mimo.%s", mimoShortName);} \
  static void bangMethod(MaxMimoT *self, t_symbol *s, short ac, t_atom *at){ \
    if(self->verbose != 0) object_error((t_object *) self, "mimo works only inside a mimo host!!!");} \
  static void listMethod(MaxMimoT *self, t_symbol *s, short ac, t_atom *at){ \
    if(self->verbose != 0) object_error((t_object *) self, "mimo works only inside a mimo host!!!");}\
  static void intMethod(MaxMimoT *self, long i){ \
    if(self->verbose != 0) object_error((t_object *) self, "mimo works only inside a mimo host!!!");} \
  static void floatMethod(MaxMimoT *self, double f){ \
    if(self->verbose != 0) object_error((t_object *) self, "mimo works only inside a mimo host!!!");}\
  void ext_main(void *r) { \
    t_class *c = class_new("mimo." mimoName, (method) new_max_object, (method) free_max_object, (long) sizeof(MaxMimoT), 0L, A_GIMME, 0); \
    class_addmethod(c, (method)helpnameMethod, "helpname", A_CANT, 0);\
    class_addmethod(c, (method)bangMethod, "bang", 0);\
    class_addmethod(c, (method)listMethod, "list", A_GIMME, 0);\
    class_addmethod(c, (method)intMethod, "int", A_LONG, 0);\
    class_addmethod(c, (method)floatMethod, "float", A_FLOAT, 0);\
    CLASS_ATTR_LONG(c, "verbose", 0, MaxMimoT, verbose); \
    CLASS_ATTR_STYLE_LABEL(c, "verbose", 0, "onoff", "Print Errors and Warning"); \
    CLASS_ATTR_SAVE(c, "verbose", 0); \
    class_register(CLASS_BOX, c); \
    max_ ## mimoClass ## _class = c; }

#endif // PIPO_MAX_WITH_DOC

#endif //_MAX_MIMO_
