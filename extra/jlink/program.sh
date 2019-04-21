openocd -f $1 -c init -c "reset halt" -c "flash write_image erase $2" -c "verify_image $2" -c "reset run" -c shutdown
