#include "csf2file.h"

bool CSF2File::load(const QString &filePath)
{
    QMutexLocker locker(&mutex);
    sampleCount=0;
    if (!SF2Enabler.ReadSFBFile(QString(filePath.toUtf8()).toStdString()))
    {
        banks.clear();
        path.clear();
        return false;
    }
    path=filePath;
    data=static_cast<short*>(SF2Enabler.sampleData.data());
    banks.clear();
    programHeaders=SF2Enabler.GetPresetHdrs();
    for (uint i=0;i<programHeaders.size();i++)
    {
        const sfPresetHdr hdr=programHeaders[i];
        banks[hdr.wPresetBank].presets[hdr.wPresetNum].assign(int(i),const_cast<char*>(hdr.achPresetName));
    }
    sampleCount=SF2Enabler.sampleData.size();
    return true;
}


