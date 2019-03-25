import MySQLdb
import glob
import os

def disconnectdb(db):
    db.commit()
    db.close()
    return



def query(cursor):

    #result_cnt = cursor.execute("select `sys_id` from `syscall_map` where `syscall`= '"+s+"'")
    #if(result_cnt==0):
        #print(s)
    sql = "INSERT INTO `log` (`deviceID`, `year`,`month`,`date`,`hour`,`minute`,`second`,`cf`,`cfID`) VALUES ('"+ str(int(devID,16)) +"','"+ str(year) + "','" + str(month) + "','"+ str(date) + "','"+ str(hour) + "','"+ str(minute)+ "','"+ str(second) + "','"+ str(cf) + "','"+ str(cfID) + "';"
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

