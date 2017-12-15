/*  Compile with ::                                                                                                                                          */
/* g++ -O0 -std=c++11 -rdynamic -Wunused-variable -ltinfo -lncurses -lpanel -lboost_thread -lboost_filesystem -lboost_system -ldl -lpthread -o lspf lspf.cpp */

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


map<string, cuaType> cuaAttrName =
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
  { "WASL",    WASL   },
  { "CHAR",    CHAR   },
  { "DATAIN",  DATAIN },
  { "DATAOUT", DATAOUT},
  { "GRPBOX",  GRPBOX },
  { "OUTPUT",  OUTPUT },
  { "TEXT",    TEXT   }, } ;


map<string, unsigned int> usrAttrNames =
{ { "N_RED",     N_RED     },
  { "N_GREEN",   N_GREEN   },
  { "N_YELLOW",  N_YELLOW  },
  { "N_BLUE",    N_BLUE    },
  { "N_MAGENTA", N_MAGENTA },
  { "N_TURQ",    N_TURQ    },
  { "N_WHITE",   N_WHITE   },
  { "B_RED",     B_RED     },
  { "B_GREEN",   B_GREEN   },
  { "B_YELLOW",  B_YELLOW  },
  { "B_BLUE",    B_BLUE    },
  { "B_MAGENTA", B_MAGENTA },
  { "B_TURQ",    B_TURQ    },
  { "B_WHITE",   B_WHITE   },
  { "R_RED",     R_RED     },
  { "R_GREEN",   R_GREEN   },
  { "R_YELLOW",  R_YELLOW  },
  { "R_BLUE",    R_BLUE    },
  { "R_MAGENTA", R_MAGENTA },
  { "R_TURQ",    R_TURQ    },
  { "R_WHITE",   R_WHITE   },
  { "U_RED",     U_RED     },
  { "U_GREEN",   U_GREEN   },
  { "U_YELLOW",  U_YELLOW  },
  { "U_BLUE",    U_BLUE    },
  { "U_MAGENTA", U_MAGENTA },
  { "U_TURQ",    U_TURQ    },
  { "U_WHITE",   U_WHITE   },
  { "P_RED",     P_RED     },
  { "P_GREEN",   P_GREEN   },
  { "P_YELLOW",  P_YELLOW  },
  { "P_BLUE",    P_BLUE    },
  { "P_MAGENTA", P_MAGENTA },
  { "P_TURQ",    P_TURQ    },
  { "P_WHITE",   P_WHITE   } } ;


set<cuaType> cuaAttrUnprot =
{ { LEF    },
  { CEF    },
  { NEF    },
  { DATAIN } } ;


map<int, unsigned int> usrAttr =
{ { N_RED,     RED     },
  { N_GREEN,   GREEN   },
  { N_YELLOW,  YELLOW  },
  { N_BLUE,    BLUE    },
  { N_MAGENTA, MAGENTA },
  { N_TURQ,    TURQ    },
  { N_WHITE,   WHITE   },

  { B_RED,     RED     | A_BOLD    },
  { B_GREEN,   GREEN   | A_BOLD    },
  { B_YELLOW,  YELLOW  | A_BOLD    },
  { B_BLUE,    BLUE    | A_BOLD    },
  { B_MAGENTA, MAGENTA | A_BOLD    },
  { B_TURQ,    TURQ    | A_BOLD    },
  { B_WHITE,   WHITE   | A_BOLD    },

  { R_RED,     RED     | A_REVERSE },
  { R_GREEN,   GREEN   | A_REVERSE },
  { R_YELLOW,  YELLOW  | A_REVERSE },
  { R_BLUE,    BLUE    | A_REVERSE },
  { R_MAGENTA, MAGENTA | A_REVERSE },
  { R_TURQ,    TURQ    | A_REVERSE },
  { R_WHITE,   WHITE   | A_REVERSE },

  { U_RED,     RED     | A_UNDERLINE },
  { U_GREEN,   GREEN   | A_UNDERLINE },
  { U_YELLOW,  YELLOW  | A_UNDERLINE },
  { U_BLUE,    BLUE    | A_UNDERLINE },
  { U_MAGENTA, MAGENTA | A_UNDERLINE },
  { U_TURQ,    TURQ    | A_UNDERLINE },
  { U_WHITE,   WHITE   | A_UNDERLINE },

  { P_FF,      GREEN   }, } ;

