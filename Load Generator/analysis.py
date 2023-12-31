import sys
import subprocess
import matplotlib
matplotlib.use('Agg')

import matplotlib.pyplot as plt

fig,axis=plt.subplots(2)
users=10
itr=40
throug=list()
avg_RTT=[]
noofuser=list()
for i in range(itr):
    test = subprocess.Popen(["./load_gen","localhost","80",str(users),"0.1","60"], stdout=subprocess.PIPE)
    output = test.communicate()[0]
    ab=output.decode("ascii")
    print(ab)
    
    noofuser.append(users)
    throug.append(float(ab.split("\n")[6].split(":")[1].strip()))
    avg_RTT.append(float(ab.split("\n")[7].split(":")[1].strip()))
    print(throug)
    axis[0].plot(noofuser,throug)
    axis[0].set_title("Throughput")
    axis[1].plot(noofuser,avg_RTT)
    axis[1].set_title("Avg RTT")

    plt.tight_layout()

    plt.savefig("throughput_graph.png")
    # plt2.savefig("average_rtt_graph.png")
    # if i < 9:
    #     users+=50
    # elif i >= 9 and i < 15:
    #     users+=100
    # else:
    users+=20
    