Repository has 6 files

1. start_run.py to start all the services 
2. stop_run.py to stop all the services
3. To run start_run.py use the command python3 start_tun.py
4. The logs wrt each services will be appended to the respective paths given in start_run.py

Instructions to be followed while running start_run.py

1. Open IarmDaemon.py & cecDaemon.py file and in the working directory variable, update the relative path till the act-hdmicecsource-test-poc (the path to this folder) from your local VM in the variables directory_iarm & directory_cec_daemon respectively

2.Open Thunder.py file and update the path of WPEFramework/config.json in the variable directory_thunder

3. Open start_run.py file , check if all the scripts that are called here are present in the same directory hierarchy.

 