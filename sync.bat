"C:\Program Files (x86)\WinSCP\WinSCP.com" /command ^
    "open sftp://matanlotem@nova.cs.tau.ac.il" ^
    "option batch continue" ^
    "synchronize -delete -filemask=""|SPCBIR; .settings/; Debug/; Release/; .git/; *.feats; *.o; ~*"" remote C:\Users\Matan\Dropbox\Study\S-3A\SoftwareProject\SPFinalProject /specific/a/home/cc/students/csguests/matanlotem/Study/SoftwareProject/SPFinalProject" ^
    "keepuptodate -delete -filemask=""|SPCBIR; .settings/; Debug/; Release/; .git/; *.feats; *.o; ~*"" C:\Users\Matan\Dropbox\Study\S-3A\SoftwareProject\SPFinalProject /specific/a/home/cc/students/csguests/matanlotem/Study/SoftwareProject/SPFinalProject" ^
    exit