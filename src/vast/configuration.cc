#include "vast/configuration.h"
#include "vast/file_system.h"
#include "vast/logger.h"
#include "vast/detail/type_manager.h"
#include "vast/util/color.h"

namespace vast {

std::string configuration::banner() const
{
  std::stringstream ss;
  auto colorize = ! check("log.no-colors");
  if (colorize)
    ss << util::color::red;

  ss << "     _   _____   __________\n"
        "    | | / / _ | / __/_  __/\n"
        "    | |/ / __ |_\\ \\  / /\n"
        "    |___/_/ |_/___/ /_/  ";

  if (colorize)
    ss << util::color::yellow;

  ss << VAST_VERSION;

  if (colorize)
    ss << util::color::reset;

  return ss.str();
}

void configuration::initialize()
{
  auto& general = create_block("general options");
  general.add('h', "help", "display this help");
  general.add('d', "directory", "VAST directory").init("vast");
  general.add('z', "advanced", "show advanced options");
  general.add("version", "print the version of VAST");

  auto& caf = create_block("CAF options", "caf");
  caf.add("threads", "number of worker threads in scheduler").single();
  caf.add("throughput", "maximum number of messages per worker invocation").single();
  caf.visible(false);

  auto min = 0;
  auto max = VAST_LOG_LEVEL;
  auto range = '[' + std::to_string(min) + '-' + std::to_string(max) + ']';

  auto& log = create_block("logger options", "log");
  log.add('v', "console", "console verbosity " + range).init(std::min(3, max));
  log.add('V', "file", "log file verbosity " + range).init(std::min(4, max));
  log.add("no-colors", "don't use colors for console output");
  log.add("function-names", "log function names");

  auto& prof = create_block("profiler options", "profiler");
  prof.add("interval", "profiling granularity in seconds").init(1);
  prof.add("rusage", "enable rusage profiling");
  prof.add("cpu", "enable gperftools CPU profiling");
  prof.add("heap", "enable gperftools heap profiling");
  prof.visible(false);

  auto& act = create_block("actor options");
  act.add('C', "core", "spawn all core actors");
  act.add(/* 'R', */ "receiver", "spawn the receiver");
  act.add(/* 'A', */ "archive", "spawn the archive");
  act.add(/* 'X', */ "index", "spawn the index");
  act.add(/* 'D', */ "identifier", "spawn the identifier");
  act.add(/* 'S', */ "search", "spawn the search");
  act.add('E', "exporter", "spawn the exporter").single();
  act.add('I', "importer", "spawn the importer").single();
  act.add('Q', "console", "spawn the query console");

  auto& imp = create_block("import options", "import");
  imp.add('s', "schema", "the schema to use for the generated events").single();
  imp.add('r', "read", "path to input file/directory").init("-");
  imp.add('i', "interface", "name of interface to read packets from").single();
  imp.add("compression", "the compression method for chunks").init("lz4");
  imp.add("batch-size", "number of events to ingest in one run").init(5000);
  imp.add("sniff-schema", "print the log schema and exit");
  imp.add("pcap-cutoff", "forego intra-flow packets after this many bytes").single();
  imp.add("pcap-maxflows", "number of concurrent flows to track").init(1000000);
  imp.add("test-id", "the base event ID").init(0);
  imp.add("test-events", "number of events to generate").init(100);
  imp.visible(false);

  auto& exp = create_block("export options", "export");
  exp.add('l', "limit", "maximum number of results").init(0);
  exp.add('q', "query", "the query string").single();
  exp.add('w', "write", "path to output file/directory").init("-");
  exp.add("pcap-flush", "flush to disk after this many packets").init(10000);
  exp.visible(false);

  auto& recv = create_block("receiver options", "receiver");
  recv.add("host", "hostname/address of the receiver").init("127.0.0.1");
  recv.add("port", "TCP port of the receiver").init(42000);
  recv.visible(false);

  auto& arch = create_block("archive options", "archive");
  arch.add("max-segment-size", "maximum segment size in MB").init(128);
  arch.add("max-segments", "maximum segments cached in memory").init(10);
  arch.add("host", "hostname/address of the archive").init("127.0.0.1");
  arch.add("port", "TCP port of the archive").init(42003);
  arch.visible(false);

  auto& idx = create_block("index options", "index");
  idx.add("batch-size", "number of events to index in one run").init(5000);
  idx.add('e', "max-events", "maximum number of events per partition").init(1 << 20);
  idx.add('p', "max-parts", "maximum number of partitions in memory").init(10);
  idx.add('a', "active-parts", "number of active partitions").init(5);
  idx.add("rebuild", "delete and rebuild index from archive");
  idx.add("host", "hostname/address of the archive").init("127.0.0.1");
  idx.add("port", "TCP port of the index").init(42004);
  idx.visible(false);

  auto& track = create_block("indentifier options", "identifier");
  track.add("host", "hostname/address of the identifier").init("127.0.0.1");
  track.add("port", "TCP port of the ID identifier").init(42002);
  track.visible(false);

  auto& srch = create_block("search options", "search");
  srch.add("host", "hostname/address of the archive").init("127.0.0.1");
  srch.add("port", "TCP port of the search").init(42001);
  srch.visible(false);

  add_conflict("console", "core");
  add_conflict("console", "identifier");
  add_conflict("console", "archive");
  add_conflict("console", "index");
  add_conflict("console", "importer");
  add_conflict("console", "exporter");
  add_conflict("console", "search");
  add_conflict("console", "receiver");

  add_dependency("import.schema", "importer");
  add_dependency("import.read", "importer");
  add_dependency("import.interface", "importer");
  add_dependency("import.sniff-schema", "importer");
  add_dependency("import.pcap-cutoff", "importer");
  add_dependency("import.pcap-maxflows", "importer");
  add_conflict("import.read", "import.interface");
  add_conflict("import.schema", "import.sniff-schema");

  add_dependency("export.limit", "exporter");
  add_dependency("export.query", "exporter");
  add_dependency("exporter", "export.query");
  add_dependency("export.write", "exporter");
  add_dependency("export.pcap-flush", "exporter");
  add_conflict("importer", "exporter");
  add_conflict("receiver", "exporter");
  add_conflict("identifier", "exporter");
}

} // namespace vast
