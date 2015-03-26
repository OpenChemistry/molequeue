MoleQueue
=========
![MoleQueue][MoleQueueLogo]

Introduction
------------

MoleQueue is an open-source, cross-platform, system-tray resident desktop
application for abstracting, managing, and coordinating the execution of tasks
both locally and on remote computational resources. Users can set up local and
remote queues that describe where the task will be executed. Each queue can
have programs, with templates to facilitate the execution of the program. Input
files can be staged, and output files collected using a standard interface.
Some highlights:

* Open source distributed under the liberal 3-clause BSD license
* Cross platform with nightly builds on Linux, Mac OS X and Windows
* Intuitive interface designed to be useful to whole community
* Support for local executation and remote schedulers (SGE, PBS, SLURM)
* System tray resident application managing queue of queues and job lifetime
* Simple, lightweight JSON-RPC 2.0 based communication over local sockets
* Qt 5 client library for simple integration in Qt applications

![Open Chemistry project][OpenChemistryLogo]
![Kitware, Inc.][KitwareLogo]

MoleQueue is being developed as part of the [Open Chemistry][OpenChemistry]
project at [Kitware][Kitware], along with companion tools and libraries to
support the work.

Installing
----------

We provide nightly binaries built by our [dashboards][Dashboard] for Mac OS
X and Windows. If you would like to build from source we recommend that you
follow our [building Open Chemistry][Build] guide that will take care of
building most dependencies.

Contributing
------------

Our project uses the standard GitHub pull request process for code review
and integration. Please check our [development][Development] guide for more
details on developing and contributing to the project. The GitHub issue
tracker can be used to report bugs, make feature requests, etc.

Our [wiki][Wiki] is used to document features, flesh out designs and host other
documentation. Our API is [documented using Doxygen][Doxygen] with updated
documentation generated nightly. We have several [mailing lists][MailingLists]
to coordinate development and to provide support.

  [MoleQueueLogo]: http://openchemistry.org/files/logos/molequeue.png "MoleQueue"
  [OpenChemistry]: http://openchemistry.org/ "Open Chemistry Project"
  [OpenChemistryLogo]: http://openchemistry.org/files/logos/openchem128.png "Open Chemistry"
  [Kitware]: http://kitware.com/ "Kitware, Inc."
  [KitwareLogo]: http://www.kitware.com/img/small_logo_over.png "Kitware"
  [Dashboard]: http://cdash.openchemistry.org/index.php?project=MoleQueue "MoleQueue Dashboard"
  [Build]: http://wiki.openchemistry.org/Build "Building MoleQueue"
  [Development]: http://wiki.openchemistry.org/Development "Development guide"
  [Wiki]: http://wiki.openchemistry.org/ "Open Chemistry wiki"
  [Doxygen]: http://doc.openchemistry.org/molequeue/api/ "API documentation"
  [MailingLists]: http://openchemistry.org/mailing-lists "Mailing Lists"
