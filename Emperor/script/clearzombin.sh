
#清楚小僵尸进程
ps -A -o stat,ppid,pid,cmd | grep -e '^[Zz]' | awk '{print $2}' | xargs kill -9

ps -ef | grep defunct | more

ps aux  | grep -e './client' | awk '{print $2}' | xargs kill -9


#查看进程打开的文件句柄数量
lsof -n|awk '{print $2}'|sort|uniq -c|sort -nr | grep 6312