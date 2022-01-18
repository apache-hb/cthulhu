REPOS = python3-dev python3-setuptools python3-pip \
                    build-essential make automake git \
                    flex bison libglib2.0-dev libpixman-1-dev \
                    gcc-11 g++-11 llvm-12 clang-12 \
					gcc-11-plugin-dev libstdc++-11-dev \
					ninja-build

AFLPP_DIR = $(shell pwd)/aflpp
AFLFC_DIR = $(shell pwd)/aflfc

update-apt:
	add-apt-repository -y ppa:ubuntu-toolchain-r/test

install-apt: update-apt
	apt install -y $(REPOS)

update-alternatives: install-apt
	update-alternatives --install /usr/bin/gcc gcc $(shell which gcc-11) 100
	update-alternatives --install /usr/bin/g++ g++ $(shell which g++-11) 100

	update-alternatives --install /usr/bin/llvm-config llvm-config $(shell which llvm-config-12) 100
	update-alternatives --install /usr/bin/clang clang $(shell which clang-12) 100
	update-alternatives --install /usr/bin/clang++ clang++ $(shell which clang++-12) 100

	update-alternatives --install /usr/bin/cc cc $(shell which gcc-11) 100
	update-alternatives --install /usr/bin/c++ c++ $(shell which g++-11) 100

$(AFLPP_DIR): install-apt
	[ -d $(AFLPP_DIR) ] || git clone https://github.com/AFLplusplus/AFLplusplus $(AFLPP_DIR) --depth 1

$(AFLFC_DIR): install-apt
	[ -d $(AFLFC_DIR) ] || git clone https://github.com/agroce/afl-compiler-fuzzer $(AFLFC_DIR) --depth 1

build-aflpp: | $(AFLPP_DIR)
	$(MAKE) -C $(AFLPP_DIR) source-only
	$(MAKE) -C $(AFLPP_DIR) install

build-aflfc: | $(AFLFC_DIR)
	$(MAKE) -C $(AFLFC_DIR)
	$(MAKE) -C $(AFLFC_DIR) install

update-bashrc:
	echo "export AFL_CRASH_EXITCODE=99" >> ~/.bashrc

install-meson: install-apt
	pip3 install meson

all: update-bashrc build-aflpp build-aflfc install-meson
