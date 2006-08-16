#!/bin/env python

# The modal is: data ("catch port channel \n") are coming throw stdin
# we have to measure the noise rate for an individual channel and

from optparse import OptionParser
import random,math
import threading,sys,time

class Channel:
    delta_max    = 128
    rate_desired = 5000
    def __init__(self,threshold):
        self.counter      = 0
        self.delta        = self.delta_max/8
        self.counter_prev = 0
        self.threshold    = threshold

    def analyze(self):
        # Check for a small too small or too big rates
        if (self.counter==0 and self.counter_prev==0) or \
           (self.counter>self.rate_desired*10 and self.counter_prev>Channel.rate_desired*10):
            if self.delta<self.delta_max:
                self.delta *= 2
                print 'speed up!',self.delta

        # Check for slow changes
        if abs(self.counter-self.counter_prev)/float(self.counter+self.counter_prev+1)>0.8:
            if self.delta>1:
                self.delta /= 2
                print 'slow down!',self.delta

        # Check for slow changes
        if abs(self.counter-self.counter_prev)/float(self.counter+self.counter_prev+1)<0.1:
            if self.delta<self.delta_max:
                self.delta *= 2
                print 'speed up!',self.delta

        print 'rate=(%d,%d)  ' % (self.counter_prev,self.counter),self.threshold,'+',self.delta,'==>>',
        if self.counter>self.rate_desired:
            self.threshold -= self.delta
        else:
            self.threshold += self.delta
        print self.threshold
        
        self.counter_prev = self.counter
    
## Generate a channel with noise from the channels list
def generator(channels):
    for i in range(100):
        k  = channels.keys()[random.randint(0,len(channels)-1)]
        ch = channels[k]
        try:
            p = 1/(1+math.exp(0.1*(ch.gen_thr-ch.threshold)))
        except:
            #print ch.gen_thr,ch.thr
            p = 0
        if p>random.random():
            i = 0
            return k,ch
    raise 'generator(): too many iterations'


def calculations(channels,lock):
    while True:
        time.sleep(1)
        #lock.acquire()
        #print 'calculations!'
        
        print '================================================='
        for k,ch in channels.items():
            print k
            ch.analyze()
            ch.counter = 0
        
        #lock.release()

if __name__ == '__main__':

    parser = OptionParser()
    parser.add_option('-g', '--generate', dest='generate',
                      default=False,action='store_true',
                      help='generate channels flow')
    parser.add_option('', '--rate', dest='rate',type='int',
                      default=5000,help='default noise rate')
    parser.add_option('-c','--catch',action='append', type='int',help='Catch number')

    (options, args) = parser.parse_args()

    channels={}

    if options.rate:
        Channel.rate_desired = options.rate

    if options.catch:
        for catch in options.catch:
            for port in range(1):
                for chan in range(6):
                    channels[(catch,port,chan)] = Channel(-1000)

    if options.generate:
        for ch in channels.values():
            ch.gen_thr   = random.randint(-1600,-400)
            ch.threshold = random.randint(-1600,-400)
        while True:
            k,ch = generator(channels)
            print k[0],k[1],k[2]

        sys.exit(0)


    lock = threading.Lock()
    thread_calc = threading.Thread(group=None,target=calculations,args=(channels,lock))
    thread_calc.setDaemon(True)
    thread_calc.start()

    while True:
        f = raw_input()
        catch,port,num = f.split()
        ch_id=(int(catch),int(port),int(num))
        #lock.acquire()
        try:
            ch = channels[ch_id]
        except:
            print 'NEW CHANNEL:',f
            ch = Channel()
            channels[ch_id] = ch
        ch.counter += 1
        #lock.release()


def idea():
    satisfied = False
    tmp_files = []
    for i in range(2):
        tmp_files.append(mkstemp('tmp_','thr'))
    while not satisfied:
        tmp_file = None
        os.system('')
