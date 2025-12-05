################################################################################
# © Robert Bosch GmbH 2024 All rights reserved, also regarding any disposal, 
# exploitation, reproduction, editing, distribution, as well as in the event 
# of applications for industrial property rights. The communication of its 
# contents to others without express authorization is prohibited. Offenders 
# will be held liable for the payment of damages. All rights reserved in the 
# event of the grant of a patent, utility model or design. 
#
#
# Project Makefile
#
################################################################################
# Project Specific Defines
################################################################################
GPIO_PIN               := 0
NAME                   := thesis
HOME                   := $(CURDIR)
ROOT_DIR               := $(HOME)
INCLUDEPATHS           := -I$(HOME)
SRC                    :=  $(wildcard *.c)
C_PROJ_FLAGS           := -DPROJECT_NAME=\"$(NAME)\"


$(info Building $(NAME))
$(info $(ROOT_DIR))
$(info $(HOME))
$(info $(INCLUDEPATHS))
$(info $(SRC))
$(info $(C_PROJ_FLAGS))

# Include the common Makefile
include ../Makefile

