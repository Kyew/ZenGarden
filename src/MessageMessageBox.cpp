/*
 *  Copyright 2009,2010,2011 Reality Jockey, Ltd.
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

#include "MessageMessageBox.h"
#include "PdGraph.h"

#define RES_BUFFER_LENGTH 64

MessageObject *MessageMessageBox::newObject(PdMessage *initString, PdGraph *graph) {
  return new MessageMessageBox(initString->getSymbol(0), graph);
}

/*
 * The message box is overloaded with many kinds of functionality.
 * A) The simplest case is one where only one message is specified, including a list of primitives
 * which should be included in one message. The list may also include variable indicies (in the form
 * of, e.g. $1, $2, etc.) which refer to those locations in the incoming message which triggers
 * the message box.
 * B) A slightly more complicated case is where several messages in the form of A) are separated
 * by a comma (','). Each of the messages is processed and sent independently from the message box
 * when it is triggered.
 * C) The most complex case is where messages in the form of A) are separated by a semicolon (';').
 * The first symbol is the name of a message receiver. The remainder of the string is converted
 * into a message.
 */
MessageMessageBox::MessageMessageBox(char *initString, PdGraph *graph) : MessageObject(1, 1, graph) {
  messageString = "";
  set(initString);
}

MessageMessageBox::~MessageMessageBox() {
  clear();
}

void MessageMessageBox::processMessage(int inletIndex, PdMessage *message) {
  if (message->isSymbol(0, "set")) {
    clear();
    if (message->getNumElements() > 1) {
      set(std::string(message->toString()).substr(strlen("set") + 1).c_str());
    }
  } else if (message->isSymbol(0, "add")) {
    add(std::string(message->toString()).substr(strlen("add") + 1).c_str());
  } else if (message->isSymbol(0, "add2")) {
    add2(std::string(message->toString()).substr(strlen("add2") + 1).c_str());
  } else if (message->isSymbol(0, "addcomma")) {
    addComma();
  } else if (message->isSymbol(0, "addsemi")) {
    addSemi();
  } else if (message->isSymbol(0, "adddollar")) {
    addDollar(std::string(message->toString()).substr(strlen("adddollar") + 1).c_str());
  } else if (message->isSymbol(0, "adddollsym")) {
    addDollarSymbol(std::string(message->toString()).substr(strlen("adddollsym") + 1).c_str());
  } else {
    char resolvedName[RES_BUFFER_LENGTH]; // resolution buffer for named destination
  
    // NOTE(mhroth): if any message has more than 64 elements, that's very bad
    PdMessage *outgoingMessage = PD_MESSAGE_ON_STACK(64);
  
    // send local messages
    for (int i = 0; i < localMessageList.size(); i++) {
      PdMessage *messageTemplate = localMessageList.at(i);
      int numElements = messageTemplate->getNumElements();
      outgoingMessage->initWithTimestampAndNumElements(message->getTimestamp(), numElements);
    
      memcpy(outgoingMessage->getElement(0), messageTemplate->getElement(0), numElements*sizeof(MessageAtom));
      for (int i = 0; i < numElements; i++) {
        if (messageTemplate->isSymbol(i)) {
          char *buffer = (char *) alloca(RES_BUFFER_LENGTH * sizeof(char));
          // TODO(mhroth): resolve string, but may be in stack buffer
          PdMessage::resolveString(messageTemplate->getSymbol(i), message, 1, buffer, RES_BUFFER_LENGTH);
          outgoingMessage->parseAndSetMessageElement(i, buffer); // buffer is resolved to float or string
        }
      }
      sendMessage(0, outgoingMessage);
    }

    // send remote messages
    for (int i = 0; i < remoteMessageList.size(); i++) {
      MessageNamedDestination namedDestination = remoteMessageList.at(i);

      PdMessage::resolveString(namedDestination.first, message, 1, resolvedName, RES_BUFFER_LENGTH);
    
      PdMessage *messageTemplate = namedDestination.second;
      int numElements = messageTemplate->getNumElements();
      outgoingMessage->initWithTimestampAndNumElements(message->getTimestamp(), numElements);
      memcpy(outgoingMessage->getElement(0), messageTemplate->getElement(0), numElements*sizeof(MessageAtom));
      for (int i = 0; i < numElements; i++) {
        if (messageTemplate->isSymbol(i)) {
          char *buffer = (char *) alloca(RES_BUFFER_LENGTH * sizeof(char));
          // TODO(mhroth): resolve string, but may be in stack buffer
          PdMessage::resolveString(messageTemplate->getSymbol(i), message, 1, buffer, RES_BUFFER_LENGTH);
          outgoingMessage->setSymbol(i, buffer);
        }
      }
      graph->sendMessageToNamedReceivers(resolvedName, outgoingMessage);
    }
  }
}

void MessageMessageBox::set(const char *str) {
  std::vector<std::string> messageStringList;
  std::vector<std::string> localMessageStringList;
  std::vector<std::string>::iterator localMessageString;
  std::string workingMsgString;
  
  messageString = str;
  //Removing last ';'
  if (messageString.size() >= 2 && messageString[messageString.size() - 1] == ';' &&
    messageString[messageString.size() - 2] != '\\') {
    messageString = messageString.substr(0, messageString.size() - 1);
  } else if (messageString == ";") {
    messageString = "";
  }

  messageStringList = StaticUtils::tokenizeString(messageString.c_str(), "\\;");
  localMessageStringList = StaticUtils::tokenizeString(messageStringList[0].c_str(), "\\,");
  
  //Parsing local messages
  for (localMessageString = localMessageStringList.begin(); localMessageString != localMessageStringList.end();
    ++localMessageString) {
    int maxElements = (localMessageString->size() / 2) + 1 ;
    PdMessage *localMessage = PD_MESSAGE_ON_STACK(maxElements);
    char *localMessageCString;

    localMessageCString = strdup(localMessageString->c_str());
    localMessage->initWithString(0.0f, maxElements, localMessageCString);
    localMessageList.push_back(localMessage->copyToHeap());
    delete localMessageCString;
  }
  
  //Parsing local messages
  for (int i = 1; i < messageStringList.size(); ++i) {
    std::string remoteMessageString = messageStringList[i];
    std::string remoteMessageReceiver;
    char *remoteMessageCString;
    int maxElements = (messageString.size() / 2) + 1;
    PdMessage *remoteMessage = PD_MESSAGE_ON_STACK(maxElements);
    MessageNamedDestination namedDestination;
    
    remoteMessageString = std::string(remoteMessageString, remoteMessageString.find_first_not_of(" "));
    remoteMessageReceiver = std::string(remoteMessageString, 0, messageString.find(" "));
    remoteMessageString = std::string(remoteMessageString, remoteMessageString.find(" ") + 1);
    remoteMessageCString = strdup(remoteMessageString.c_str());
    remoteMessage->initWithString(0.0, maxElements, remoteMessageCString);
    namedDestination = make_pair(StaticUtils::copyString(remoteMessageReceiver.c_str()), remoteMessage->copyToHeap());
    remoteMessageList.push_back(namedDestination);
    delete remoteMessageCString;
  }
}

void MessageMessageBox::clear() {
  // delete the message list and all of the messages in it
  for (int i = 0; i < localMessageList.size(); i++) {
    localMessageList.at(i)->freeMessage();
  }
  localMessageList.clear();

  // delete the remote message list
  for (int i = 0; i < remoteMessageList.size(); i++) {
    MessageNamedDestination namedDestination = remoteMessageList.at(i);
    free(namedDestination.first);
    namedDestination.second->freeMessage();
  }

  remoteMessageList.clear();
  messageString = "";
}

void MessageMessageBox::add(const char *str) {
  messageString += (std::string(str) + "\\;");
  set(messageString.c_str());
}

void MessageMessageBox::add2(const char *str) {
  messageString += str;
  set(messageString.c_str());
}

void MessageMessageBox::addComma() {
  messageString += ",";
  set(messageString.c_str());
}

void MessageMessageBox::addSemi() {
  messageString += "\\;";
  set(messageString.c_str());
}

void MessageMessageBox::addDollar(const char *str) {
  if (!StaticUtils::isNumeric(str)) {
    graph->printErr("MessageMessageBox: bad arguments for adddollar command");
    return;
  }
  messageString += std::string("$") + str;
  set(messageString.c_str());
}

void MessageMessageBox::addDollarSymbol(const char *str) {
  messageString += std::string("$") + str;
  set(messageString.c_str());
}
