#include "jdk_world.h"
#include "jdk_settings.h"
#include "if2k_mini_config.h"

#if IF2K_IS_SERVER
#define IF2K_MINI_SERVER_BIND "0:8000"
#else
#define IF2K_MINI_SERVER_BIND "localhost:8000"
#endif

jdk_setting_description if2k_mini_kernel_defaults[] =
{
{"if2k.version", IF2K_MINI_VERSION, "Version #" },

#if JDK_IS_MACOSX && JDK_IS_DEBUG
  {"daemon.name", IF2K_MINI_SERVER_SERVICE_NAME, "Name of daemon"},
  {"home", "/Users/jeffk/if2k", "Home directory"},
  {"pid.dir", "~", "Directory for pid file"},
  {"daemon", "0", "Daemonize"},
  {"daemon.uid", "0", "User id to run subprocesses as" },
  {"daemon.gid", "0", "Group id to run subprocesses as" },
  {"log.type", "3", "Logging type: 0=none, 1=syslog, 2=stderr, 3=file, 4=console"},
  {"log.file", "~/if2kd.log", "Logging file name." },
  {"log.detail", "8", "Logging detail level: 0=Errors, 1=Warnings, 2=Notices, 3=Info, 4-8=Debug" },


#elif JDK_IS_MACOSX && !JDK_IS_DEBUG
  {"daemon.name", IF2K_MINI_SERVER_SERVICE_NAME, "Name of daemon"},
  {"home", "/Library/If2k", "Home directory"},
  {"pid.dir", "/var/run", "Directory for pid file"},
  {"daemon.uid", "0", "User id to run subprocesses as" },
  {"daemon.gid", "0", "Group id to run subprocesses as" },
#if 1
{"daemon", "0", "Daemonize"}, // launchd handles daemonizing for us
  {"log.type", "3", "Logging type: 0=none, 1=syslog, 2=stderr, 3=file, 4=console"},
  {"log.file", "/var/log/if2kd.log", "Logging file name." },
  {"log.detail", "2", "Logging detail level: 0=Errors, 1=Warnings, 2=Notices, 3=Info, 4-8=Debug" },
#else
{"daemon", "0", "Daemonize"},
{"log.type", "2", "Logging type: 0=none, 1=syslog, 2=stderr, 3=file, 4=console"},
{"log.file", "/var/log/if2kd.log", "Logging file name." },
{"log.detail", "6", "Logging detail level: 0=Errors, 1=Warnings, 2=Notices, 3=Info, 4-8=Debug" },
#endif

#elif JDK_IS_WIN32 && JDK_IS_DEBUG
  {"daemon.name", IF2K_MINI_SERVER_SERVICE_NAME, "Name of daemon"},
  {"home", "~/if2k", "Home directory"},
  {"log.type", "3", "Logging type: 0=none, 1=event log, 2=stderr, 3=file"},
  {"log.file", "log.txt", "Logging file name." },
  {"log.detail", "8", "Logging detail level: 0=Errors, 1=Warnings, 2=Notices, 3=Info, 4-8=Debug" },


#elif JDK_IS_WIN32 && !JDK_IS_DEBUG
  {"daemon.name", IF2K_MINI_SERVER_SERVICE_NAME, "Name of daemon"},
  {"home", "~/if2k", "Home directory"},
  {"log.type", "3", "Logging type: 0=none, 1=event log, 2=stderr, 3=file"},
  {"log.file", "log.txt", "Logging file name." },
  {"log.detail", "2", "Logging detail level: 0=Errors, 1=Warnings, 2=Notices, 3=Info, 4-8=Debug" },


#elif JDK_IS_LINUX && JDK_IS_DEBUG
  {"daemon.name", IF2K_MINI_SERVER_SERVICE_NAME, "Name of daemon"},
  {"home", "~/if2k", "Home directory" },
  {"pid.dir", "~/if2k", "Directory for pid file"},
  {"daemon.uid", "0", "User id to run subprocesses as" },
  {"daemon.gid", "0", "Group id to run subprocesses as" },
  {"log.type", "3", "Logging type: 0=none, 1=syslog, 2=stderr, 3=file, 4=console"},
  {"log.file", "~/if2k/if2kd.log", "Logging file name." },
  {"log.detail", "8", "Logging detail level: 0=Errors, 1=Warnings, 2=Notices, 3=Info, 4-8=Debug" },


#elif JDK_IS_LINUX && !JDK_IS_DEBUG
  {"daemon.name", IF2K_MINI_SERVER_SERVICE_NAME, "Name of daemon"},
  {"home", "/etc/if2k", "Home directory"},
  {"pid.dir", "/var/run", "Directory for pid file"},
  {"daemon.uid", "0", "User id to run subprocesses as" },
  {"daemon.gid", "0", "Group id to run subprocesses as" },
  {"log.type", "3", "Logging type: 0=none, 1=syslog, 2=stderr, 3=file, 4=console"},
  {"log.file", "/var/log/if2kd.log", "Logging file name." },
  {"log.detail", "2", "Logging detail level: 0=Errors, 1=Warnings, 2=Notices, 3=Info, 4-8=Debug" },


#else
#error unknown platform defs
#endif


  {"license.name", "Evaluation copy", "Licensed to" },
  {"license.email", "", "Email address" },
  {"license.address", "", "Address" },
  {"license.city", "", "City" },
  {"license.region", "", "Region/Country" },
  {"license.zipcode", "", "Zip/postal code" },
  {"license.phone", "", "Telephone #" },
  {"license.website", "www.internetfilter.com", "Website" },
  {"license.info", "demo", "Type of license" },
  {"license.expires", "", "Expiry date" },
  {"license.serial", "", "License serial number" },
  {"license.expires.code", "", "Expiry code in seconds since epoch" },
  {"license.valid", "1", "License validity" },  

  {"product.id", "The Internet Filter IF-2K", "Product ID" },  

  {"http.proxy", "", "IP address and port of further http proxy"},
  {"http.proxy.auth.basic.enable", "0", "Enable basic auth for further http proxy"},
  {"http.proxy.auth.basic.username", "", "username for basic auth for further http proxy"},
  {"http.proxy.auth.basic.password", "", "password for basic auth for further http proxy"},
  {"http.proxy.image", "", "IP address and port of further http proxy for images only"},

  {"nntp.server", "", "IP address and port of further nntp server"},

  {"kernel.settings.license.local", "license.txt", "Local filename for license file"},
  {"kernel.settings.master.local", "install.txt", "Local filename for master settings"},

  {"kernel.settings.additional.local", "additional.txt", "Local filename for client specific settings"},
  {"kernel.settings.additional.remote", "", "Remote URL for client specific settings"},
  {"kernel.settings.update.rate", "0", "Settings update rate in minutes"},
  {"kernel.settings.refresh.rate", "5", "Settings update rate in minutes for first load after startup"},
  
  {"kernel.blocking.enable", "1", "Enable blocking" },
  {"kernel.blocking.referer.enable", "1", "Allow unknown sites that are referred to by good sites" },
  {"kernel.blocking.block.bad", "1", "Block known bad sites" },
  {"kernel.blocking.block.unknown", "0", "Block unknown sites" },
  {"kernel.blocking.image.redirect.enable", "1", "Redirect images on bad sites" },
  {"kernel.blocking.image.redirect.jpg.url", "imageblock.jpg", "jpg replacement image url" },
  {"kernel.blocking.image.redirect.gif.url", "imageblock.gif", "gif replacement image url" },
  {"kernel.blocking.image.redirect.png.url", "imageblock.png", "png replacement image url" },
  {"kernel.blocking.image.redirect.randomize", "0", "enable random image replacement" },
  {"kernel.blocking.censor.url.content", "1", "Enable blocking if content contains a bad url" },
  {"kernel.blocking.censor.phrase.content", "1", "Enable blocking if content contains a bad phrase" },
  {"kernel.blocking.block.url.content", "1", "Enable blocking if content contains a bad url" },
  {"kernel.blocking.block.phrase.content", "1", "Enable blocking if content contains a bad phrase" },
  {"kernel.blocking.content.match.threshold", "4", "Content match threshold to stop transferring content" },
  {"kernel.blocking.nntp.redirect", "comp.os.faq", "NNTP group to redirect bad requests to" },

  {"kernel.log.enable", "0", "Enable access logging" },
  {"kernel.log.file.enable", "0", "Enable access logging to file" },
  {"kernel.log.file.local", "log.txt", "local filename to log to" },
  {"kernel.log.email.enable", "0", "Enable logging to email address" },
  {"kernel.log.email.recipient", "", "email address to send email to" },
  {"kernel.log.email.from", "", "email address to send email from" },
  {"kernel.log.email.smtp", "", "smtp server to use for sending email" },
  {"kernel.log.email.subject", "Internet Filter Report", "Subject line of email message" },
  {"kernel.log.email.timeout", "60", "Number of seconds to wait before emailing message" },

  {"kernel.log.all", "0", "Log all accesses" },
  {"kernel.log.bad", "0", "Log bad website accesses" },
  {"kernel.log.unknown", "0", "Log unknown website accesses" },
  {"kernel.log.overridden", "0", "Log overridden website accesses" },
  {"kernel.log.good", "0", "Log good website accesses" },
  {"kernel.log.images", "0", "Log image url accesses" },
  {"blockpage.title", "Blocked", "Message to use for title of block page"},
  {"blockpage.blockedmessage", "Blocked by the Internet Filter", "Message to print below image on block page" },
  {"blockpage.url", "block.html", "URL for block page" },
  {"blockpage.image.url", "blocked1.jpg", "Image for block page" },
  {"blockpage.image.link", "http://www.internetfilter.com/", "Link on block page" },
  
  {"kernel.override.allow", "0", "Allow override feature" },
  {"kernel.override.now", "0", "Do override now" },
  {"kernel.override.unknown.only", "0", "Enable override mode on unknown sites only" },
  {"kernel.remote.update.enable", "1", "Allow remote updates via http" },
  {"kernel.remote.update.password", "9e5c209f0ada27b58a4f40dbba90b37c2f342fc0", "Password for remote updates via http" },

  {"kernel.db.precompiled.enable", "1", "Enable precompiled database" },
  {"kernel.db.precompiled.dir", "predb", "Local directory for precompiled database" },
  {"kernel.db.precompiled.remote", "", "Remote URL for precompiled database" },
  {"kernel.db.user.dir", "db", "Local directory for user database" },
  {"kernel.db.user.remote", "", "Remote URL for user database" },
  {"kernel.db.user.remote.cache.dir", "cache", "Filename prefix for cached remote files" },

  {"kernel.db.category.1.enable", "1", "" },
  {"kernel.db.category.2.enable", "0", "" },
  {"kernel.db.category.3.enable", "0", "" },
  {"kernel.db.category.4.enable", "1", "" },
  {"kernel.db.category.5.enable", "0", "" },
  {"kernel.db.category.6.enable", "0", "" },
  {"kernel.db.category.7.enable", "0", "" },
  {"kernel.db.category.8.enable", "0", "" },

  {"kernel.db.category.1.name", "Pornography", "" },
  {"kernel.db.category.2.name", "Guns/Violence", "" },
  {"kernel.db.category.3.name", "Hate speech", "" },
  {"kernel.db.category.4.name", "Gambling", "" },
  {"kernel.db.category.5.name", "Gay support", "" },
  {"kernel.db.category.6.name", "Jobs", "" },
  {"kernel.db.category.7.name", "Dating", "" },
  {"kernel.db.category.8.name", "Webmail/Chat", "" },

  {"kernel.db.good.category.1.enable", "1", "" },
  {"kernel.db.good.category.2.enable", "1", "" },
  {"kernel.db.good.category.3.enable", "1", "" },
  {"kernel.db.good.category.4.enable", "1", "" },
  {"kernel.db.good.category.5.enable", "1", "" },
  {"kernel.db.good.category.6.enable", "1", "" },
  {"kernel.db.good.category.7.enable", "1", "" },
  {"kernel.db.good.category.8.enable", "1", "" },

  {"kernel.db.good.category.1.name", "1", "" },
  {"kernel.db.good.category.2.name", "2", "" },
  {"kernel.db.good.category.3.name", "3", "" },
  {"kernel.db.good.category.4.name", "4", "" },
  {"kernel.db.good.category.5.name", "5", "" },
  {"kernel.db.good.category.6.name", "6", "" },
  {"kernel.db.good.category.7.name", "7", "" },
  {"kernel.db.good.category.8.name", "8", "" },
  
  {"nntpfilter.interface", "", "IP address and port to bind to for NNTP filter"},
  {"nntpfilter.max.sockets", "0", "Maximum number of simultaneous incoming connections supported for NNTP filter" },
  {"nntpfilter.max.requests", "300", "" },
  {"nntpfilter.max.time", "36000", "" },

    {"httpfilter.interface", IF2K_MINI_SERVER_BIND, "IP address and port to bind to for HTTP filter"},

  {"httpfilter.visible.interface", "", "IP address and port that external programs use to access. Blank means same as httpfilter.interface"},
  {"httpfilter.max.sockets", "100", "Maximum number of simultaneous incoming connections supported for HTTP filter" },
  {"httpfilter.max.requests", "200", "" },
  {"httpfilter.max.time", "36000", "" },

  {"httpfilter.max.retries", "4", "Maximum retries when connecting to external server"},
  {"httpfilter.retry.delay", "100", "Time in milliseconds before retrying connection to external server"},
  {"httpfilter.server.webdir", "web", "Local directory for web files"},
  {"httpfilter.server.error.404", "error404.html", "URL for 404 error page" },
  {"httpfilter.server.error.connection", "error_connection.html", "URL for connection error page" },
  {"httpfilter.scanner.queue.size", "32768", "Number of bytes to scan before streaming text content." },

  {0,0}
};
