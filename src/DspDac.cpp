/*
 *  Copyright 2009,2012 Reality Jockey, Ltd.
 *                 info@rjdj.me
 *                 http://rjdj.me/
 * 
 *  This file is part of ZenGarden.
 *
 *  ZenGarden is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ZenGarden is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with ZenGarden.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ArrayArithmetic.h"
#include "DspDac.h"
#include "PdGraph.h"
#include "PdContext.h"

MessageObject *DspDac::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspDac(initMessage, graph);
}

DspDac::DspDac(PdMessage *initMessage, PdGraph *graph)
  : DspObject(0, 1, 0, 0, graph) {

  // empty init message will always contains a bang (cf. PdMessage::initWithString)
  if (initMessage->isBang(0)) {
    outputBuffers.reserve(graph->getNumOutputChannels());
    for (int i = 0; i < graph->getNumOutputChannels(); ++i)
      outputBuffers.push_back(graph->getGlobalDspBufferAtOutlet(i));
  } else {
    outputBuffers.reserve(initMessage->getNumElements());
    for (int i = 0; i < initMessage->getNumElements(); ++i) {
      int index;
      
      if (initMessage->isFloat(i)) {
        index = static_cast<int>(initMessage->getFloat(i)) - 1;
        if (index < graph->getContext()->getNumOutputChannels() && index >= 0) {
          outputBuffers.push_back(graph->getGlobalDspBufferAtOutlet(index));
        } else {
          outputBuffers.push_back(NULL);
        }
      } else {
        graph->printErr("DspDac: init message should only contain output channel indices");
        outputBuffers.clear();
        return;
      }
    }
  }
  incomingDspConnections = std::vector<std::list<ObjectLetPair> >(outputBuffers.size());
}

DspDac::~DspDac() {}

void DspDac::processDspWithIndex(int fromIndex, int toIndex) {
  if (outputBuffers.size() == 0)
    return;
  for (int i = 0; i < outputBuffers.size(); ++i) {
    if (outputBuffers.at(i) != NULL) {
      ArrayArithmetic::add(outputBuffers.at(i), dspBufferAtInlet[i], outputBuffers.at(i), fromIndex, toIndex);
    }
  }
}
