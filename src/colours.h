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

using namespace lspf ;

namespace lspfc
{

map<attType, unsigned int> cuaAttr ;

map<string, int> colourName =
{ { "RED",     RED     },
  { "GREEN",   GREEN   },
  { "YELLOW",  YELLOW  },
  { "BLUE",    BLUE    },
  { "MAGENTA", MAGENTA },
  { "PINK",    MAGENTA },
  { "TURQ",    TURQ    },
  { "WHITE",   WHITE   } } ;


map<int, string> colourValue =
{ { 1, "R" },
  { 2, "G" },
  { 3, "Y" },
  { 4, "B" },
  { 5, "M" },
  { 6, "T" },
  { 7, "W" } } ;


map<string, attType> cuaAttrName =
{ { "AB",   AB   },
  { "ABSL", ABSL },
  { "ABU",  ABU  },
  { "AMT",  AMT  },
  { "AWF",  AWF  },
  { "CT",   CT   },
  { "CEF",  CEF  },
  { "CH",   CH   },
  { "DT",   DT   },
  { "ET",   ET   },
  { "EE",   EE   },
  { "FP",   FP   },
  { "FK",   FK   },
  { "IMT",  IMT  },
  { "IWF",  IWF  },
  { "LEF",  LEF  },
  { "LID",  LID  },
  { "LI",   LI   },
  { "NEF",  NEF  },
  { "NT",   NT   },
  { "PI",   PI   },
  { "PIN",  PIN  },
  { "PS",   PS   },
  { "PT",   PT   },
  { "PAC",  PAC  },
  { "PUC",  PUC  },
  { "RP",   RP   },
  { "SI",   SI   },
  { "SC",   SC   },
  { "SAC",  SAC  },
  { "SUC",  SUC  },
  { "VOI",  VOI  },
  { "WMT",  WMT  },
  { "WASL", WASL },
  { "WT",   WT   } } ;


map<string, attType> noncuaAttrName =
{ { "CHAR",    CHAR    },
  { "DATAIN",  DATAIN  },
  { "DATAOUT", DATAOUT },
  { "INPUT",   INPUT   },
  { "OUTPUT",  OUTPUT  },
  { "TEXT",    TEXT    } } ;


set<attType> attrUnprot = { INPUT, EE, LEF, CEF, NEF, DATAIN } ;

}
