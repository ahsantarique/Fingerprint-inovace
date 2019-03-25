from __future__ import print_function
import random
import os
import struct
import MySQLdb
import glob


def disconnectdb(db):
    db.commit()
    db.close()
    return



def query(cursor):

    #result_cnt = cursor.execute("select `sys_id` from `syscall_map` where `syscall`= '"+s+"'")
    #if(result_cnt==0):
        #print(s)
    sql = "INSERT INTO `log` (`deviceID`, `year`, `month`, `date`, `hour`, `minute`, `second`, `cf`, `cfID`) VALUES ('"+ str(int(devID,16)) +"','"+ str(year) + "','" + str(month) + "','"+ str(date) + "','"+ str(hour) + "','"+ str(minute)+ "','"+ str(second) + "','"+ str(cf) + "','"+ str(cfID) + "');"
    print(sql)
    cursor.execute(sql)
    #cursor.execute("select `sys_id` from `syscall_map` where `syscall`= '"+s+"'")

##    result = cursor.fetchone()
##    sys_id=result[0]
##    syscall_count[sys_id] += 1
##
##    f2.write(str(sys_id)+"\n")
##    prev_sys_id = sys_id
##
##    cursor.execute("SELECT max(`sys_id`) FROM `syscall_map`;");
##    mapped_so_far=  cursor.fetchone()[0]


def database_work():

    db = MySQLdb.connect(host="localhost", port=3306, user="root", passwd="", db="fingerprint")
    cursor = db.cursor()
    query(cursor)
    disconnectdb(db)


piece_size= 22
with open("outLog.txt", "wb") as outFile:
    with open("LOG.txt", "rb") as inFile:
        while True:
            piece = inFile.read(piece_size)

            if piece == "":
                break # end of file
            deviceID = os.urandom(4)
            outFile.write(deviceID)
            outFile.write(piece)
            devID = deviceID.encode('hex')
            print('device ID: %s = ' %devID + str(int(devID,16)))

            
            log = piece.encode('hex')
            year = int(log[0:2], 16)
            month = int(log[2:4], 16)
            date = int(log[4:6], 16)
            hour = int(log[6:8], 16)
            minute = int(log[8:10], 16)
            second = int(log[10:12], 16)
            cf = int(log[12:14], 16)
            cfID = int(log[14:], 16)
            print(log);
            print("%d %d %d %d %d %d %d %s" %(year,month,date,hour,minute,second,cf,cfID))

            database_work()            
            
        outFile.write(struct.pack('1B', 0xff))


