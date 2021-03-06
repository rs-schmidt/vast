#include <caf/all.hpp>
#include "vast.h"

int main(int argc, char *argv[])
{
  auto cfg = vast::configuration::parse(argc, argv);
  if (! cfg)
  {
    std::cerr << cfg.error() << ", try -h or -z" << std::endl;
    return 1;
  }

  if (argc < 2 || cfg->check("help") || cfg->check("advanced"))
  {
    cfg->usage(std::cerr, cfg->check("advanced"));
    return 0;
  }
  else if (cfg->check("version"))
  {
    std::cout << VAST_VERSION << std::endl;
    return 0;
  }

  vast::announce_builtin_types();

  auto initialized = vast::logger::instance()->init(
      *vast::logger::parse_level(*cfg->get("log.console")),
      *vast::logger::parse_level(*cfg->get("log.file")),
      ! cfg->check("log.no-colors"),
      cfg->check("log.function-names"),
      vast::path{*cfg->get("directory")}.complete() / "log");

  if (! initialized)
  {
    std::cerr << "failed to initialize logger" << std::endl;
    return 1;
  }

  VAST_LOG_VERBOSE(" _   _____   __________");
  VAST_LOG_VERBOSE("| | / / _ | / __/_  __/");
  VAST_LOG_VERBOSE("| |/ / __ |_\\ \\  / / ");
  VAST_LOG_VERBOSE("|___/_/ |_/___/ /_/  " << VAST_VERSION);
  VAST_LOG_VERBOSE("");

  auto threads = std::thread::hardware_concurrency();
  if (auto t = cfg->as<size_t>("caf.threads"))
    threads = *t;

  auto throughput = std::numeric_limits<size_t>::max();
  if (auto t = cfg->as<size_t>("caf.throughput"))
    throughput = *t;

  caf::set_scheduler<>(threads, throughput);
  VAST_LOG_VERBOSE("set scheduler threads to " << threads);
  VAST_LOG_VERBOSE("set scheduler maximum throughput to " <<
                   (throughput == std::numeric_limits<size_t>::max()
                    ? "unlimited" : std::to_string(throughput)));

  auto program = caf::spawn<vast::program>(std::move(*cfg));
  caf::anon_send(program, caf::atom("run"));
  caf::await_all_actors_done();
  caf::shutdown();

  vast::cleanup();

  return 0;
}
