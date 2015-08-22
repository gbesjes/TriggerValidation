import ROOT

SAMPLES = {
    'Z': 'mc15_13TeV.361108.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Ztautau.merge.AOD.e3601_s2576_s2132_r6630_r6264',
    'EB': 'data15_13TeV.00271421.physics_EnhancedBias.merge.AOD.r6913_p2346',
}


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
