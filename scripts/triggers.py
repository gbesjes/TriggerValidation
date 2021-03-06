import ROOT
__all__ = [
    'list_to_vector'
]


L1_ITEMS = [
    # TAUS
    'L1_TAU8', 'L1_TAU12', 'L1_TAU12IL', 'L1_TAU12IM',
    'L1_TAU12IT', 'L1_TAU20', 'L1_TAU20IL', 'L1_TAU20IM',
    'L1_TAU20IT', #'L1_TAU25IT', 
    'L1_TAU30','L1_TAU40','L1_TAU60',
    # JETS
    'L1_J12', 'L1_J25',
    # MUONS
    'L1_MU10',
    # MET
    'L1_XE35', 'L1_XE45',
    # EM
    'L1_EM15', 'L1_EM15HI',
    # TAU-TAU / TAU-TAU-JET / TAU-TAU-XE
    'L1_TAU20IM_2TAU12IM', 'L1_TAU20_2TAU12',
    # 'L1_TAU25IT_2TAU12IT_2J25_3J12',
    'L1_TAU20IL_2TAU12IL_J25_2J20_3J12',
    'L1_TAU20IM_2TAU12IM_J25_2J20_3J12',
    'L1_TAU20_2TAU12_J25_2J20_3J12',
    'L1_TAU20IM_2TAU12IM_XE35',
    'L1_TAU20_2TAU12_XE35',
    'L1_TAU20IM_2TAU12IM_XE40',
    # EM-TAU / EM-TAU-JET
    'L1_EM15HI_2TAU12IM', 'L1_EM15HI_2TAU12IM_J25_3J12',
    'L1_EM15HI_2TAU12_J25_3J12', 'L1_EM15HI_TAU40_2TAU15',
    # MU-TAU / MU-TAU-JET
    'L1_MU10_TAU12', 'L1_MU10_TAU12IM',
    'L1_MU10_TAU12IM_J25_2J12', 'L1_MU10_TAU12IL_J25_2J12', 
    'L1_MU10_TAU12_J25_2J12','L1_MU10_TAU20', 'L1_MU10_TAU20IM',
    # TAU-XE-JET
    'L1_TAU20IM_2J20_XE45', 'L1_TAU20_2J20_XE45',
    'L1_TAU25_2J20_XE45', 'L1_TAU20IM_2J20_XE50',
    # EM-TAU-XE
    'L1_EM15HI_2TAU12IM_XE35',
    'L1_EM15HI_2TAU12_XE35',
    # MU-TAU-XE
    'L1_MU10_TAU12IM_XE35',
    'L1_MU10_TAU12IT_XE35',
    'L1_MU10_TAU12IM_XE40',
    # TOPOLOGICAL TRIGGERS
    'L1_J25_3J12_EM15-TAU12I',
    'L1_DR-MU10TAU12I_TAU12I-J25',
    'L1_J25_2J12_DR-MU10TAU12I', 
    # 'L1_J25_2J20_3J12_BOX-TAU20ITAU12I', #  Someone should implement the box :-/
    'L1_J25_2J20_3J12_DR-TAU20ITAU12I',
    'L1_MU10_TAU12I-J25',
    'L1_XE45_TAU20-J20',
    'L1_XE35_EM15-TAU12I',
    'L1_XE40_EM15-TAU12I',
    'L1_DR-MU10TAU12I',
    'L1_TAU12I-J25',
    'L1_EM15-TAU40',
    'L1_TAU20-J20',
    'L1_EM15-TAU12I',
    'L1_EM15TAU12I-J25',
    'L1_DR-EM15TAU12I-J25',
    'L1_TAU20ITAU12I-J25',
    'L1_DR-TAU20ITAU12I',
    # 'L1_BOX-TAU20ITAU12I', # Someone should implement the box properly :-/
    'L1_DR-TAU20ITAU12I-J25',
    ]


HLT_ITEMS = [
    'HLT_tau25_perf_tracktwo',
    'HLT_tau25_medium1_tracktwo',
    'HLT_tau35_perf_tracktwo',
    'HLT_tau35_medium1_tracktwo',
]


def list_to_vector(l, data_type='std::string'):
    vec = ROOT.vector(data_type)()
    for a in l:
        vec.push_back(a)
    return vec

