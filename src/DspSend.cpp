/*
 *  Copyright 2010 Reality Jockey, Ltd.
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

#include "DspSend.h"
#include "PdGraph.h"

DspSend::DspSend(PdMessage *initMessage, PdGraph *graph) : DspObject(0, 1, 0, 0, graph) {
  if (initMessage->isSymbol(0)) {
    name = StaticUtils::copyString(initMessage->getSymbol(0));
    
    // as this object has no official outlet buffer, reuse the class variable in order to reference
    // the local scratch buffer.
    dspBufferAtOutlet0 = (float *) calloc(blockSizeInt, sizeof(float));
  } else {
    name = NULL;
    graph->printErr("send~ not initialised with a name.");
  }
}

DspSend::~DspSend() {
  free(name);
  free(dspBufferAtOutlet0);
  dspBufferAtOutlet0 = NULL;
}

const char *DspSend::getObjectLabel() {
  return "send~";
}

ObjectType DspSend::getObjectType() {
  return DSP_SEND;
}

char *DspSend::getName() {
  return name;
}

float **DspSend::getBuffer() {
  return &dspBufferAtOutlet0;
}

void DspSend::processDsp() {
  if (incomingDspConnections[0].size() > 1) {
    resolveInputBuffers(0, dspBufferAtOutlet0);
  } else {
    memcpy(dspBufferAtOutlet0, dspBufferAtInlet0, numBytesInBlock);
  }
}
