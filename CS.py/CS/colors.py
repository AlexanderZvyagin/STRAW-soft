BLACK = ''
RED = ''
GREEN = ''
YELLOW = ''
BLUE = ''
MAGENTA = ''
CYAN = ''
WHITE = ''
RESET = ''
BOLD = ''
REVERSE = ''

BLACKBG = ''
REDBG = ''
GREENBG = ''
YELLOWBG = ''
BLUEBG = ''
MAGENTABG = ''
CYANBG = ''
WHITEBG = ''

def colors_enable():
    global BLACK,RED,GREEN,YELLOW,BLUE,MAGENTA,CYAN,WHITE,RESET,BOLD,REVERSE
    global BLACKBG,REDBG,GREENBG,YELLOWBG,BLUEBG,MAGENTABG,CYANBG,WHITEBG

    BLACK = '\033[30m'
    RED = '\033[31m'
    GREEN = '\033[32m'
    YELLOW = '\033[33m'
    BLUE = '\033[34m'
    MAGENTA = '\033[35m'
    CYAN = '\033[36m'
    WHITE = '\033[37m'

    RESET = '\033[0;0m'
    BOLD = '\033[1m'
    REVERSE = '\033[2m'

    BLACKBG = '\033[40m'
    REDBG = '\033[41m'
    GREENBG = '\033[42m'
    YELLOWBG = '\033[43m'
    BLUEBG = '\033[44m'
    MAGENTABG = '\033[45m'
    CYANBG = '\033[46m'
    WHITEBG = '\033[47m'

def colors_disable():
    global BLACK,RED,GREEN,YELLOW,BLUE,MAGENTA,CYAN,WHITE,RESET,BOLD,REVERSE
    global BLACKBG,REDBG,GREENBG,YELLOWBG,BLUEBG,MAGENTABG,CYANBG,WHITEBG

    BLACK = ''
    RED = ''
    GREEN = ''
    YELLOW = ''
    BLUE = ''
    MAGENTA = ''
    CYAN = ''
    WHITE = ''
    RESET = ''
    BOLD = ''
    REVERSE = ''

    BLACKBG = ''
    REDBG = ''
    GREENBG = ''
    YELLOWBG = ''
    BLUEBG = ''
    MAGENTABG = ''
    CYANBG = ''
    WHITEBG = ''

# Enable colors by default
colors_enable()
