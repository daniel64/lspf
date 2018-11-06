/*
  Copyright (c) 2015 Daniel John Erdos

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

namespace lspfc
{
map<string, attType> cuaAttrName =
{ { "AB",      AB     },
  { "ABSL",    ABSL   },
  { "ABU",     ABU    },
  { "AMT",     AMT    },
  { "AWF",     AWF    },
  { "CT",      CT     },
  { "CEF",     CEF    },
  { "CH",      CH     },
  { "DT",      DT     },
  { "ET",      ET     },
  { "EE",      EE     },
  { "FP",      FP     },
  { "FK",      FK     },
  { "IMT",     IMT    },
  { "LEF",     LEF    },
  { "LID",     LID    },
  { "LI",      LI     },
  { "NEF",     NEF    },
  { "NT",      NT     },
  { "PI",      PI     },
  { "PIN",     PIN    },
  { "PT",      PT     },
  { "PS",      PS     },
  { "PAC",     PAC    },
  { "PUC",     PUC    },
  { "RP",      RP     },
  { "SI",      SI     },
  { "SAC",     SAC    },
  { "SUC",     SUC    },
  { "VOI",     VOI    },
  { "WMT",     WMT    },
  { "WT",      WT     },
  { "WASL",    WASL   } } ;


map<string, attType> noncuaAttrName =
{ { "CHAR",    CHAR    },
  { "DATAIN",  DATAIN  },
  { "DATAOUT", DATAOUT },
  { "INPUT",   INPUT   },
  { "OUTPUT",  OUTPUT  },
  { "TEXT",    TEXT    } } ;


map<string, unsigned int> usrAttrNames =
{ { "N_RED",      N_RED      },
  { "N_GREEN",    N_GREEN    },
  { "N_YELLOW",   N_YELLOW   },
  { "N_BLUE",     N_BLUE     },
  { "N_MAGENTA",  N_MAGENTA  },
  { "N_TURQ",     N_TURQ     },
  { "N_WHITE",    N_WHITE    },
  { "B_RED",      B_RED      },
  { "B_GREEN",    B_GREEN    },
  { "B_YELLOW",   B_YELLOW   },
  { "B_BLUE",     B_BLUE     },
  { "B_MAGENTA",  B_MAGENTA  },
  { "B_TURQ",     B_TURQ     },
  { "B_WHITE",    B_WHITE    },
  { "R_RED",      R_RED      },
  { "R_GREEN",    R_GREEN    },
  { "R_YELLOW",   R_YELLOW   },
  { "R_BLUE",     R_BLUE     },
  { "R_MAGENTA",  R_MAGENTA  },
  { "R_TURQ",     R_TURQ     },
  { "R_WHITE",    R_WHITE    },
  { "BR_RED",     BR_RED     },
  { "BR_GREEN",   BR_GREEN   },
  { "BR_YELLOW",  BR_YELLOW  },
  { "BR_BLUE",    BR_BLUE    },
  { "BR_MAGENTA", BR_MAGENTA },
  { "BR_TURQ",    BR_TURQ    },
  { "BR_WHITE",   BR_WHITE   },
  { "U_RED",      U_RED      },
  { "U_GREEN",    U_GREEN    },
  { "U_YELLOW",   U_YELLOW   },
  { "U_BLUE",     U_BLUE     },
  { "U_MAGENTA",  U_MAGENTA  },
  { "U_TURQ",     U_TURQ     },
  { "U_WHITE",    U_WHITE    } } ;


set<attType> attrUnprot = { INPUT, LEF, CEF, NEF, DATAIN } ;

}
