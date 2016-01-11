#include "AudioBinaryFile.h"

AudioBinaryFile::AudioBinaryFile(const std::string &filepath) {
  _audioBinaryFile = new juce::File(filepath);

  if (_audioBinaryFile->existsAsFile()) {
    juce::FileInputStream *inputStream = _audioBinaryFile->createInputStream();

    if (inputStream == NULL)
      return;

    int numberOfFiles = inputStream->readInt();

    for (int i = 0; i < numberOfFiles; ++i) {
      //Getting file header index
      int headerIndex = inputStream->readInt();
      //Saving current position in file
      int positionInGlobalHeader = inputStream->getPosition();
      //Setting position to read the filepath size and filepath in the file header
      inputStream->setPosition(headerIndex + 2 * sizeof(int));
      int filepathSize = inputStream->readInt();
      char *filepath = new char[filepathSize + 1];
      inputStream->read(filepath, filepathSize);
      filepath[filepathSize] = 0;
      //Adding the data to the map
      _audioFilesHeadersIndices[filepath] = headerIndex;
      delete filepath;
      //Returning to position in global header
      inputStream->setPosition(positionInGlobalHeader);
    }
    delete inputStream;
  }
}

AudioBinaryFile::~AudioBinaryFile() {

}

juce::InputStream *AudioBinaryFile::getFileInputStream(const std::string &filepath) 
{
  if (_audioFilesHeadersIndices.find(filepath) == _audioFilesHeadersIndices.end())
  {
    return NULL;
  }
  juce::FileInputStream *inputStream = _audioBinaryFile->createInputStream();
  juce::MemoryInputStream *result = NULL;

  if (inputStream == NULL)
    return NULL;

  inputStream->setPosition(_audioFilesHeadersIndices[filepath]);
  int dataIndex = inputStream->readInt();
  int dataSize = inputStream->readInt();

  inputStream->setPosition(dataIndex);
  char *buffer = new char[dataSize];
  if (dataSize == inputStream->read(buffer, dataSize)) //if not file is corrupted and we return null
  {
    result = new MemoryInputStream(buffer, dataSize, true);
  }
  delete buffer;
  delete inputStream;
  return result;
}