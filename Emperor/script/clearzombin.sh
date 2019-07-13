
#清楚小僵尸进程
ps -A -o stat,ppid,pid,cmd | grep -e '^[Zz]' | awk '{print $2}' | xargs kill -9

ps -ef | grep defunct | more

ps aux  | grep -e './client' | awk '{print $2}' | xargs kill -9