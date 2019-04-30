#!/usr/bin/env python3

import json
import subprocess
import time
    
with open('issue_accounts.json') as f:
    a = json.load(f)
    accounts = a['issues']
    
for i in range(0, len(accounts)):
    a = accounts[i]
    
    args = 'cleos push action %s issue \'["%s","%s",%s,%s,%s,"%s"]\' -p %s@owner' % (a['dist_contract'], a['issue'],a['quantity'],a['lock_flag'],a['lock_rate'],a['lock_time'],a['memo'],a['dist_contract'])
    
    if subprocess.call(args, shell=True):
        print('run:', args)
    else:
        print('run.:', args)
        
    time.sleep(1)

        