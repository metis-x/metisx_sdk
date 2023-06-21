adminId=$1
rm -rf admin_$adminId.log
while :
do
    sudo python3 cli.py stat-admin 0 | grep OutstandingCount >> admin_$adminId.log
    sleep 0.1
done
