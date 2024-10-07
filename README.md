# lspf
Open source Linux version of IBM's z/OS ISPF Dialogue Manager written in C++

Application programs wishing to use DM Services, can be written in C++ or rexx (open object rexx).  Calls are made via a pApplication method or the ISPEXEC interface.

Most services are supported, including many Library Management services, as are dynamic input/output areas, scrollable fields, tables, table displays (including multi-line models), file tailoring and a large subset of the ISPF panel language (including optional support for the *REXX/*ENDREXX and PANEXIT panel statements).  If REXX support has been enabled, simple ISPF panels can also be displayed.  See doc directory for more details.

This as been written as an exercise whilst learning C++ and OOP and I've put it on GitHub in case anyone wants to use/contribute to it.  There is a lack of applications, but there is a browser, an editor similar to the mainframe PDF editor that also supports macros (including line command macros) written in OOREXX or C++ and syntax hilighting.  There is also a file list application, a compare utility and some system utilities (including for systemd).  See options 3 and 5 from the Primary Menu.

Other features include split screen, application stacking, keylists, reflists, command tables, field expansion, field history, command retrieve, cursor-sensitive help, keylist help, swapbar, show pfkeys etc.

Screenshots:

![Alt text](https://user-images.githubusercontent.com/15121632/32369290-942fdd3e-c080-11e7-908b-379ff2acdaef.png)

![Alt text](https://user-images.githubusercontent.com/15121632/32378901-d889eb50-c0a3-11e7-83fe-3ee00460cd1a.png)

![Alt text](https://user-images.githubusercontent.com/15121632/32369287-920be2d2-c080-11e7-936e-69664450d4aa.png)

![Alt text](https://user-images.githubusercontent.com/15121632/32369293-966305f4-c080-11e7-977c-269c76c0dec6.png)

