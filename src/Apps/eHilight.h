#include <string>
using namespace std ;

class hilight
{
        public:
                string   hl_language ;
                int      hl_oBrac1   ;
                int      hl_oBrac2   ;
                bool     hl_oComment ;
                hilight()
                {
                        hl_language = ""    ;
                        hl_oBrac1   = 0     ;
                        hl_oBrac2   = 0     ;
                        hl_oComment = false ;
                }
} ;

void addCppHilight( hilight &, string, string & ) ;

