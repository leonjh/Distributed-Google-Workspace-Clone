# How To Compile

1. ./delete_script
2. ./clean_build_script

# How to create the config files
1. cd bin
2. mkdir config
3. cd config
4. touch backend\
4.5 the file should be populated with the backend ip's, one per line with the associated ports
  for example the file in its entirity should look like the following 5 lines, depending on the number of tablet servers
  127.0.0.1:8001
  127.0.0.1:8002
  127.0.0.1:8003
  127.0.0.1:8004
  127.0.0.1:8005
5. touch frontend\
  5.5 this file should look exactly like the backend config file, however with different points, specifically in the 3000s.
  For example, the file in its entirity should look like the following 2 lines, depending on the number of HTTP servers
  127.0.0.1:3001
  127.0.0.1:3002

# Running the Frontend
1. cd bin
2. create config file with frontend server addresses
3. run frontend servers: `./main_server -p {port #} -v ../static`
4. run frontend load balancer: `./FrontendLoadBalancer -p 3000 -v {config file name}`
5. run smtp server: `./smtp -p {port #}`
6. Note: frontend related servers should be run after backend servers are up.

# Notes
1. ./clean_build_script clears the checkpoints and logs while build script does not.
2. If you want state to be persisted after shutting the system down, you must ensure all storage servers are up and synced with each other before shutting the system down.
3. The system should automatically tolerate (number of replicas - 1) faults.# Distributed-Google-Workspace-Clone
