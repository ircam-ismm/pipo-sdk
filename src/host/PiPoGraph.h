/** -*- mode: c++; c-basic-offset:2 -*-
 *
 * @file PiPoGraph.h
 * @author Joseph Larralde
 *
 * @brief PiPo dataflow graph class that parses a graph description string and instantiates
 * the corresponding graph of PiPos (made of PiPoSequence and PiPoParallel instances)
 *
 * PiPoGraph is built around the classes PiPoSequence, PiPoParallel and PiPoOp.
 *
 * A PiPoGraph is itself also a PiPo module, i.e. data processing works the same
 * as for a simple operator :
 * - the PiPoGraph is prepared for processing by calling streamAttributes() on it.
 * - data is piped into the PiPoGraph with the frames() method.
 *
 * @ingroup pipoapi
 *
 * @copyright
 * Copyright (C) 2017 by ISMM IRCAM - Centre Pompidou, Paris, France.
 * All rights reserved.
 *
 * License (BSD 3-clause)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _PIPO_GRAPH_
#define _PIPO_GRAPH_

#include <algorithm> // for right definition of std::remove
#include <string>
#include <vector>
#include <iostream>

#include "PiPoOp.h"
#include "PiPoSequence.h"
#include "PiPoParallel.h"

// NB : this is a work in progress
// TODO: define error return codes for parsing

class PiPoGraph : public PiPo
{
public:
  typedef enum PiPoGraphTypeE
  {
    undefined = -1, leaf = 0, sequence, parallel
  } PiPoGraphType;

private:
  bool          toplevel_;
  std::string   representation_;
  PiPoGraphType graphtype_ = undefined;

  // parallel graphs if graphtype_ is parallel
  // sequence of graphs if graphtype_ is sequence (not sequence of PiPos)
  // empty if graphtype_ is leaf
  std::vector<PiPoGraph> subgraphs_;

  // use op_ if we are a leaf to parse instanceName and to hold attributes
  PiPoOp op_;

  PiPo  *pipo_ = nullptr; // points to the pipo that links the subgraph together: seq or parallel, or the leaf pipo

  // linearised lists of contained pipos and their instance names, filled by linearize()
  std::vector<PiPo *>        pipolist_; 
  std::vector<std::string>   instancenamelist_;

  // combined list of all attrs and their descriptions of all pipos in subgraphs, filled by copyPiPoAttributes()
  std::vector<std::string *> attrnames_;
  std::vector<std::string *> attrdescrs_;
  
  PiPoModuleFactory          *modulefactory_;

public:
  PiPoGraph() = delete;
    
  PiPoGraph(PiPo::Parent *_parent, PiPoModuleFactory *_modulefactory = NULL, bool _toplevel = true)
  : PiPo(_parent)
  {
    modulefactory_ = _modulefactory;
    toplevel_      = _toplevel;
  }

  ~PiPoGraph ()
  {
    if (toplevel_)
    {
      for (unsigned int i = 0; i < attrnames_.size(); ++i)
        delete attrnames_[i];

      for (unsigned int i = 0; i < attrdescrs_.size(); ++i)
        delete attrdescrs_[i];
    }

    clear();
  }

  void clear ()
  {
    pipolist_.clear();
    instancenamelist_.clear();

    for (unsigned int i = 0; i < subgraphs_.size(); ++i)
      subgraphs_[i].clear();

    if (graphtype_ != leaf  &&  pipo_ != nullptr)
      delete pipo_;     // delete parallels and sequences instantiated by graph with new (no op)
    else
      op_.clear();      // let op clear itself (and free pipo)

    pipo_ = nullptr;
  } // end clear()
  
  bool create (std::string graphStr, bool copy_attrs = true)
  {
    if (parse(graphStr)  &&  instantiate()  &&  wire())
    {
      if (copy_attrs)
        copyPiPoAttributes();   // puts instancename before each attr (which is redone by certain hosts)
      
      linearize(this);
      return true;
    }
    else
    {
      clear();
      // we could call parent->signalError() here, but this might output messages at the wrong time
      return false;  //TODO: return error code from parsing
    }
  } // end create()

  // duplicate from other graph, reinstantiate pipos
  // (this is called to create independent graphs for each pipo process, duplicating the graph and its attributes built by host)
  bool duplicate (const PiPoGraph *other)
  {
    if (other->pipo_ == NULL)
    { // parsing had syntax error: no pipo was created, but graph is partially filled --> leave empty graph
      clear();
      // we could call parent->signalError() here, but this might output (many) messages at the wrong time, let the host handle it
      return false; //TODO: return error code from parsing
    }
    
    parent          = other->parent;     // from pipo base class
    toplevel_       = other->toplevel_;
    representation_ = other->representation_;
    modulefactory_  = other->modulefactory_; // copy pointer to singleton
    graphtype_      = other->graphtype_;
    subgraphs_      = other->subgraphs_;     // copy vector and PiPoGraph elements before reinstantiating

    // duplicate sub graphs
    for (int i = 0; i < other->subgraphs_.size(); i++)
      subgraphs_[i].duplicate(&other->subgraphs_[i]);

    // clone pipos by re-instantiating them (only for top level graph, not for subgraphs)
    switch (graphtype_)
    {
      case leaf:// for graph leafs, reinstantiate pipo in new op, copy attrs via cloneAttrs()
        op_.set(0, parent, modulefactory_, other->op_);
        pipo_ = op_.getPiPo();
      break;

      case sequence:
        pipo_ = new PiPoSequence(parent);
      break;
        
      case parallel:
        pipo_ = new PiPoParallel(parent);
      break;

      default:
        return false;
      break;
    }
    
    if (toplevel_  &&  wire()) // connect sequence/parallel subgraphs recursively
    { // now refill arrays pipolist_, instancenamelist_, attrnames_, attrdescrs_
      // don't call copyPiPoAttributes(); it will put instancename before each attr (which is redone by maxmubu host)
      linearize(this);
    }

    return true;
  } // end duplicate()

private:
  //======================== PARSE GRAPH EXPRESSION ==========================//

  bool parse(std::string graphStr)
  {
    //=================== BASIC SYNTAX RULES CHECKING ========================//

    //========== check if we have the right number of '<' and '>' ============//

    int  cnt   = 0;
    bool camel = false; // true if we have a sequence of parallel sections like in <a>b<c>

    for (unsigned int i = 0; i < graphStr.length(); ++i)
    {
      if (graphStr[i] == '<')
        cnt++;
      else if (graphStr[i] == '>')
        cnt--;

      if (cnt == 0  &&  i < graphStr.length() - 1) // we are outside a parallel section before the end of the graph
	camel = true; 
    }
    
    if (cnt != 0)
      return false; //TODO: return parse error message

    // TODO : add more syntax checking here ?

    //======= determine the type of graph (leaf, sequence or parallel) =======//

    unsigned int trims = 0;
    while (graphStr[0] == '<' && graphStr[graphStr.length() - 1] == '>'  &&  !camel)
    {
      graphStr = graphStr.substr(1, graphStr.length() - 2);
      trims++;
    }

    representation_ = graphStr;

    // by default we are a sequence
    graphtype_ = sequence;

    // if we have surrounding "<...>" and first-level commas, we are a parallel
    if (trims > 0)
    { 
      int subLevelsCnt = 0;

      for (unsigned int i = 0; i < graphStr.length(); ++i)
        if (graphStr[i] == '<')
        {
          subLevelsCnt++;
        }
        else if (graphStr[i] == '>')
        {
          subLevelsCnt--;
        }
        else if (subLevelsCnt == 0 && graphStr[i] == ',')
        {
          graphtype_ = parallel;
          break;
        }
    }

    // if we don't have any sequential / parallelism symbol, and we are not a single top-level pipo 
    // (because we need to be inside a PiPoGraph at the top-level), then we are a leaf :
    if (graphStr.find("<") >= std::string::npos &&
        graphStr.find(">") >= std::string::npos &&
        graphStr.find(",") >= std::string::npos &&
        graphStr.find(":") >= std::string::npos &&
        !toplevel_)
      graphtype_ = leaf;

    //====== now fill (or not) subgraphs_ vector according to graph type ======//

    switch (graphtype_)
    {
      //========================== leaf graph, we are a single PiPo, trim spaces
      case leaf:
      {
        // trim spaces - TODO: trim tabs and other spaces ?
        representation_.erase(
          std::remove(representation_.begin(), representation_.end(), ' '),
          representation_.end()
        );

        size_t pos = 0;
        op_.parse(representation_.c_str(), pos);

        // leaf string representation cannot be empty
        return (graphStr.length() > 0); //TODO: return parse error message if false
      }
      break;

      //================= sequence of graphs, parse according to ':', <' and '>'
      case sequence:
      {
        int subLevelsCnt = 0;
        unsigned int lastStartIndex = 0;
        std::vector< std::pair<unsigned int, unsigned int> > subStrings;

        if (graphStr[0] == '<')
          subLevelsCnt++;

        for (unsigned int i = 1; i < graphStr.length(); ++i)
        {
          if (graphStr[i] == ':' && subLevelsCnt == 0)
          {
            subStrings.push_back(std::pair<unsigned int, unsigned int>(
              lastStartIndex, i - lastStartIndex
            ));
            lastStartIndex = i + 1;
          }
          else if (graphStr[i] == '<')
          {
            if (subLevelsCnt == 0  &&  i > lastStartIndex) // avoid fail with <a><b> which would push (i, 0)
            {
              subStrings.push_back(std::pair<unsigned int, unsigned int>(
                lastStartIndex, i - lastStartIndex
              ));
              lastStartIndex = i;
            }
            subLevelsCnt++;
          }
          else if (graphStr[i] == '>')
          {
            subLevelsCnt--;
            if (subLevelsCnt == 0)
            {
              subStrings.push_back(std::pair<unsigned int, unsigned int>(
                lastStartIndex, i - lastStartIndex + 1
              ));
              lastStartIndex = i + 1;
            }
          }
          else if (graphStr[i] == ',' && subLevelsCnt == 0)
            // cannot have first-level commas in a sequence
            return false; //TODO: return parse error message
        }

        if (graphStr[graphStr.length() - 1] != '>')
        {
          subStrings.push_back(std::pair<unsigned int, unsigned int>(
            lastStartIndex, (unsigned int) graphStr.length() - lastStartIndex
          ));
        }

        for (unsigned int i = 0; i < subStrings.size(); ++i)
        {
          subgraphs_.push_back(PiPoGraph(parent, modulefactory_, false));
          PiPoGraph &g = subgraphs_[subgraphs_.size() - 1];

          if (!g.parse(graphStr.substr(subStrings[i].first, subStrings[i].second)))
            return false; //TODO: return parse error message
        }

        return (subLevelsCnt == 0); //TODO: return parse error message if false
      }
      break;

      //============================= parallel graphs, parse according to commas
      case parallel:
      {
        int subLevelsCnt = 0;
        std::vector<unsigned int> commaIndices;

        commaIndices.push_back(0);

        for (unsigned int i = 0; i < graphStr.length(); ++i)
          if (graphStr[i] == '<')
            subLevelsCnt++;
          else if (graphStr[i] == '>')
            subLevelsCnt--;
          else if (graphStr[i] == ',' && i < graphStr.length() - 1 && subLevelsCnt == 0)
            commaIndices.push_back(i + 1);

        commaIndices.push_back(static_cast<unsigned int>(graphStr.length()) + 1);

        for (unsigned int i = 0; i < commaIndices.size() - 1; ++i)
        {
          subgraphs_.push_back(PiPoGraph(parent, modulefactory_, false));
          PiPoGraph &g = subgraphs_[subgraphs_.size() - 1];
          unsigned int blockStart = commaIndices[i];
          unsigned int blockLength = commaIndices[i + 1] - blockStart - 1;

          if (!g.parse(graphStr.substr(blockStart, blockLength)))
            return false; //TODO: return parse error message
        }

        return (subLevelsCnt == 0); //TODO: return parse error message if false
      }
      break;

      //===================================================== this never happens
      default:
      break;
    }

    return false; //TODO: return parse error message
  } // end parse()

  //================ ONCE EXPRESSION PARSED, INSTANTIATE OPs =================//

  bool instantiate ()
  {
    if (graphtype_ == leaf)
    {
      if (!op_.instantiate(parent, modulefactory_))
        return false;
      
      pipo_ = op_.getPiPo();
    }
    else if (graphtype_ == sequence)
    {
      for (unsigned int i = 0; i < subgraphs_.size(); ++i)
        if (!subgraphs_[i].instantiate())
          return false;

      pipo_ = new PiPoSequence(parent);
    }
    else if (graphtype_ == parallel)
    {
      for (unsigned int i = 0; i < subgraphs_.size(); ++i)
        if (!subgraphs_[i].instantiate())
          return false;

      pipo_ = new PiPoParallel(parent);
    }

    return true;
  } // end instantiate()

  bool wire()
  {
    for (unsigned int i = 0; i < subgraphs_.size(); ++i)
      subgraphs_[i].wire();

    if (graphtype_ == sequence)
        for (unsigned int i = 0; i < subgraphs_.size(); ++i)
          static_cast<PiPoSequence *>(pipo_)->add(subgraphs_[i].getPiPo());

    else if (graphtype_ == parallel)
        for (unsigned int i = 0; i < subgraphs_.size(); ++i)
          static_cast<PiPoParallel *>(pipo_)->add(subgraphs_[i].getPiPo());

    return true;
  }

  // recursively collect attr names and descriptions from subgraphs,
  // append to attrnames_ and attrdescrs_
  // TODO: add an option to get PiPoAttributes only from named modules ?
  void copyPiPoAttributes()
  {
    for (unsigned int i = 0; i < subgraphs_.size(); ++i)
    {
      PiPoGraph &subGraph = subgraphs_[i];

      subGraph.copyPiPoAttributes();
      PiPo *pipo = subGraph.getPiPo(); /// current subgraph pipo
      unsigned int numAttrs = pipo->getNumAttrs();

      if (subGraph.getGraphType() == leaf)
      {
        std::string instanceName = subGraph.getInstanceName();

        for (unsigned int iAttr = 0; iAttr < numAttrs; ++iAttr)
        {
          PiPo::Attr *attr = pipo->getAttr(iAttr);
          std::string *attrName  = new std::string(instanceName + "." + attr->getName());
          std::string *attrDescr = new std::string(std::string(attr->getDescr()) + " (" + instanceName + ")");

          attrnames_.push_back(attrName);
          attrdescrs_.push_back(attrDescr);

          PiPo *p = toplevel_ ? this : pipo_;   // pipo to add attr to 
          p->addAttr(p, attrnames_[attrnames_.size() - 1]->c_str(), attrdescrs_[attrdescrs_.size() - 1]->c_str(), attr);
        }
      }

      if (subGraph.getGraphType() == sequence || subGraph.getGraphType() == parallel)
      {
        // aopend subgraph attrs to this graph attr list
        attrnames_.insert(attrnames_.end(), subGraph.attrnames_.begin(), subGraph.attrnames_.end());
        attrdescrs_.insert(attrdescrs_.end(), subGraph.attrdescrs_.begin(), subGraph.attrdescrs_.end());

        for (unsigned int iAttr = 0; iAttr < numAttrs; ++iAttr)
        {
          PiPo::Attr *attr = pipo->getAttr(iAttr);
          PiPo *p = toplevel_ ? this : pipo_;
          p->addAttr(p, subGraph.attrnames_[iAttr]->c_str(), subGraph.attrdescrs_[iAttr]->c_str(), attr);
        }
      }
    }
  } // end copyPiPoAttributes()

  
  // recursively collect pipos from subgraphs, put into list for indexed access by host
  void linearize (PiPoGraph *toplevel)
  {
    for (unsigned int i = 0; i < subgraphs_.size(); ++i)
    {
      PiPoGraph &subGraph = subgraphs_[i];

      subGraph.linearize(toplevel);
      PiPo *pipo = subGraph.getPiPo();

      if (subGraph.getGraphType() == leaf)
      {
        std::string instanceName = subGraph.getInstanceName();

        toplevel->pipolist_.push_back(pipo);
        toplevel->instancenamelist_.push_back(instanceName);
      }
      // else: sequence/parallel is already done in recursive call to linearize()
    }
  } // end linearize()

  
  /** @name PiPoGraph query methods */
  /** @{ */
public:
  size_t getSize()
  {
    return pipolist_.size();
  }

  int getIndex (const char *instanceName) 
  {
    for (unsigned int i = 0; i < instancenamelist_.size(); i++)
    {
      if (strcmp(instancenamelist_[i].c_str(), instanceName) == 0) // todo: use map
        return i;
    }

    return -1;
  }

  PiPo *getPiPo()
  {
    return pipo_;
  }

  PiPo *getHead()
  {
    return pipo_;
  }

  // access to linearised list of pipos in graph by index
  PiPo *getPiPo (unsigned int index)
  {
    return pipolist_[index];
  }
  
  PiPo *getPiPo (const char *instanceName)
  {
    int index = getIndex(instanceName);

    if(index >= 0)
      return getPiPo(index);

    return NULL;
  }

  const char *getInstanceName ()
  {
    return graphtype_ == leaf  ?  op_.getInstanceName()  :  "";
  }

  const char *getInstanceName (unsigned int index)
  {
    return instancenamelist_[index].c_str();
  }

  PiPoGraphType getGraphType()
  {
    return graphtype_;
  }

  std::string &getString()
  {
    return representation_;
  }

  
  //=============== OVERRIDING ALL METHODS FROM THE BASE CLASS ===============//

  void setParent(PiPo::Parent *parent) override
  {
    if (pipo_ != NULL)
      pipo_->setParent(parent);

    for (unsigned int i = 0; i < subgraphs_.size(); ++i)
      subgraphs_[i].setParent(parent);
  }

  PiPo *getReceiver(unsigned int index = 0) override
  {
    return pipo_->getReceiver(index);
  }

  void setReceiver(PiPo *receiver, bool add = false) override
  {
    if (pipo_ != NULL) // can be called by setupProcs although pipo graph is not yet wired (???)
      pipo_->setReceiver(receiver);
  }

  int reset() override
  {
    return pipo_->reset();
  }

  int segment(double time, bool start) override
  {
    return pipo_->segment(time, start);
  }

  int finalize(double inputEnd) override
  {
    return pipo_->finalize(inputEnd);
  }

  int streamAttributes(bool hasTimeTags, double rate, double offset,
                               unsigned int width, unsigned int height,
                               const char **labels, bool hasVarSize,
                               double domain, unsigned int maxFrames) override
  {
    return pipo_->streamAttributes(hasTimeTags, rate, offset,
				   width, height, labels, hasVarSize,
				   domain, maxFrames);
  }

  int frames (double time, double weight, PiPoValue *values,
                      unsigned int size, unsigned int num) override
  {
    return pipo_->frames(time, weight, values, size, num);
  }

  // void print() {
  //   std::cout << representation_ << " " << graphtype_ << std::endl;
  //   for (unsigned int i = 0; i < subgraphs_.size(); ++i) {
  //     std::cout << " ";
  //     subgraphs_[i].print();
  //   }
  // }
};

#endif /* _PIPO_GRAPH_ */
