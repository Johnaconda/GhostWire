#include "ghostwire/core/bootstrap.hpp"

int main(int argc, char **argv) {
    return ghostwire::core::bootstrap_main(ghostwire::core::Role::desktop, argc, argv);
}
