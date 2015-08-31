import ROOT



EOS_PATH = '/eos/atlas/user/q/qbuat'
EOS_SNIFFER = ROOT.SH.SampleHandler()
EOS_LIST = ROOT.SH.DiskListEOS(EOS_PATH, "root://eosatlas/" + EOS_PATH)
ROOT.SH.ScanDir().scan(EOS_SNIFFER, EOS_LIST)

def get_sample(sample_name):
    samples = filter(lambda s: s.name() == sample_name, EOS_SNIFFER)
    if len(samples) != 1:
        raise RuntimeError('The number of samples is wrong')
    handler = ROOT.SH.SampleHandler()
    handler.add(samples[0])
    return handler
