/*
       The following constants shall be defined as possible values for the priority argument of syslog():

       LOG_EMERG
              A panic condition was reported to all processes.

       LOG_ALERT
              A condition that should be corrected immediately.

       LOG_CRIT
              A critical condition.

       LOG_ERR
              An error message.

       LOG_WARNING
              A warning message.

       LOG_NOTICE
              A condition requiring special handling.

       LOG_INFO
              A general information message.

       LOG_DEBUG
              A message useful for debugging programs.
*/

#include <unistd.h>
#include <syslog.h>
#include <stdio.h>


int main(int argc, char **argv) {

	const char *ident = argv[0];
	int option = LOG_PID;
	int facility = LOG_LOCAL0;

	openlog(ident, option, facility);

	// int mask = LOG_MASK(LOG_DEBUG);
	// setlogmask(mask);

	int i = 0;
	while (1)
	{
		i++;
/* 7 */		syslog(LOG_DEBUG,	"LOG_DEBUG:%u:%s,%d",	LOG_DEBUG,	argv[1],	i);
/* 6 */		syslog(LOG_INFO,	"LOG_INFO:%u:%s,%d",	LOG_INFO,	argv[1],	i);
/* 5 */		syslog(LOG_NOTICE,	"LOG_NOTICE:%u:%s,%d",	LOG_NOTICE,	argv[1],	i);
/* 4 */		syslog(LOG_WARNING,	"LOG_WARNING:%u:%s,%d",	LOG_WARNING,	argv[1],	i);
/* 3 */		syslog(LOG_ERR,		"LOG_ERR:%u:%s,%d",	LOG_ERR,	argv[1],	i);
/* 2 */		syslog(LOG_CRIT,	"LOG_CRIT:%u:%s,%d",	LOG_CRIT,	argv[1],	i);
/* 1 */		syslog(LOG_ALERT,	"LOG_ALERT:%u:%s,%d",	LOG_ALERT,	argv[1],	i);
/* 0 */		// syslog(LOG_EMERG,	"LOG_EMERG:%u:%s,%d",	LOG_EMERG,	argv[1],	i);
		// sleep(1);
	}

	return 0;
}


/*
 * edit `/etc/rsyslog.conf'
 *

# Log local0
local0.*                                                /tmp/test_syslog.log

# Log anything (except mail) of level info or higher.
# Don't log private authentication messages!
*.info;mail.none;authpriv.none;cron.none;local0.none                /var/log/messages

# ### imuxsock begins to drop messages from pid `xxx' due to rate-limiting
$SystemLogRateLimitInterval 0


sudo service rsyslog restart
 */
