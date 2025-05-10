#include "climiter.h"

CLimiter::CLimiter()
{

}

void CLimiter::init(const int Index, QWidget* MainWindow) {
    m_Name=devicename;
    IDevice::init(Index,MainWindow);
    addJackWaveOut(jnOut);
    addJackWaveIn();
    addParameterVolume("Limit Vol");
    addParameterVolume();
    /* 80 Hz is the lowest frequency with which zero-crosses were
                 * observed to occur (this corresponds to 40 Hz signal frequency).
                 */
    // so lets make the buffer double size anyhoo :)
    buflen = presets.SampleRate / 40;

    pos = 0;
    ready_num = 0;
    ring.reserve(buflen);
    updateDeviceParameter();
}

CAudioBuffer *CLimiter::getNextA(const int ProcIndex) {
    const CMonoBuffer* InBuffer = FetchAMono(jnIn);
    if (!InBuffer->isValid()) return nullptr;
    uint run_length;
    uint total_length = 0;
    CMonoBuffer* OutBuffer=MonoBuffer(ProcIndex);

    while (total_length < m_BufferSize)
    {
        run_length = buflen;
        if (run_length + total_length > m_BufferSize)
            run_length = m_BufferSize - total_length;

        while (ready_num < run_length)
        {
            uint index_offs = 0;
            //look for zero-crossings and detect a half cycle
            if (ring.read_buffer(pos, ready_num) >= 0)
            {
                while ((ring.read_buffer(pos, ready_num + index_offs) >= 0) &&
                       (ready_num + index_offs < run_length))
                {
                    index_offs++;
                }
            }
            else
            {
                while ((ring.read_buffer(pos, ready_num + index_offs) <= 0) &&
                       (ready_num + index_offs < run_length))
                {
                    index_offs++;
                }
            }

            /* search for max value in scanned halfcycle */
            float max_value = 0;
            for (uint i = ready_num; i < ready_num + index_offs; i++)
            {
                if (fabsf(ring.read_buffer(pos, i)) > max_value)
                {
                    max_value = fabsf(ring.read_buffer(pos, i));
                }
            }
            float section_gain = 1;
            if (max_value>0) section_gain = limit_vol / max_value;
            if (max_value > limit_vol)
            {
                for (uint i = ready_num; i < ready_num + index_offs; i++)
                {
                    ring.write_buffer(ring.read_buffer(pos, i) * section_gain, pos, i);
                }
            }
            ready_num += index_offs;
        }

        /* push run_length values out of ringbuffer, feed with input */
        for (uint i = 0; i < run_length; i++)
        {
            OutBuffer->setAt(i,out_vol * ring.push_buffer(InBuffer->at(i), pos));
        }
        ready_num -= run_length;
        total_length += run_length;
    }
    //*(latency) = buflen;
    return m_AudioBuffers[ProcIndex];
}

void CLimiter::updateDeviceParameter(const CParameter* /*p*/) {
    limit_vol=m_Parameters[pnLimitVol]->PercentValue;
    out_vol=m_Parameters[pnOutVol]->PercentValue;
}
