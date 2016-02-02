/*
 *  Copyright 2015, 2016 AudioGaming
 *  
 *  This file is a part of the ZenGarden fork by AudioGaming
 *  See next comment for original copyright
 *
 */
/*
 *  Copyright 2009 Reality Jockey, Ltd.
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

#ifndef _MESSAGE_PRINT_H_
#define _MESSAGE_PRINT_H_

#include "MessageObject.h"

/** [print] */
class MessagePrint : public MessageObject {
  
  public:
    static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
    MessagePrint(PdMessage *initString, PdGraph *graph);
    ~MessagePrint();
  
    static const char *getObjectLabel();
    std::string toString();
  
  private:
    void processMessage(int inletIndex, PdMessage *message);
  
    char *name;
};

inline const char *MessagePrint::getObjectLabel() {
  return "print";
}

inline std::string MessagePrint::toString() {
  return std::string(MessagePrint::getObjectLabel()) + " " + name;
}

#endif // _MESSAGE_PRINT_H_
