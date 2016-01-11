#ifndef __AUDIOBINARYFILE_H__
#define __AUDIOBINARYFILE_H__

#include "JuceHeader.h"

#include <map>
#include <string>

/* File structure

- GLOBAL HEADER
| INT : Number of files
| INT[] : File headers indices
---------------
- File 1 Header
| INT : File data index
| INT : File size
| INT : File path size
| CHAR[] : File path
----------------
- File 1 Data
----------------
....
*/

class AudioBinaryFile
{
public:
  AudioBinaryFile(const std::string &filepath);
  ~AudioBinaryFile();

  juce::InputStream *getFileInputStream(const std::string &filepath);

private:
  AudioBinaryFile();

  juce::ScopedPointer<juce::File> _audioBinaryFile;
  std::map<std::string, unsigned int> _audioFilesHeadersIndices;
};

#endif //__AUDIOBINARYFILE_H__