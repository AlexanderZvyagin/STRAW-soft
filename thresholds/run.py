import os
def main(script,session,iterations=10):
    for i in range(iterations):
        for catch in [328,329,330,331,332]:
            if 0!=os.system('%s %d %s%d' % (script,catch,session,i) ):
                return

main('./script ','za',10)
main('./script2','zb', 5)
