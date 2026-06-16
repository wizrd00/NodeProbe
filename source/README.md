## Warnings
- `arpman_request_mac()` : the address that been passed to the `recvfrom()` is `struct sockaddr_ll` instead of `struct sockaddr_storage`.
- `arpman_request_mac()` : the `recvfrom_ret` hasn't been checked that is it equal to `sizeof(arp_inet_header_t)` or not, because I think it's unecessary and `res_header` is gonna check to make sure it has valid fields so a failed recv copies invalid data into `res_header` and it cannot pass the checks.
