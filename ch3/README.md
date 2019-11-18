# tiny ping

In order to makes `ping` work as non-root user, setcap to the app:
```
sudo setcap cap_net_raw,cap_net_admin+p myping
```
Here, `+p` is the cap flags, `Permitted`, there are two flags `Inheritable` and `Effective` which are not used in this case. Another capabilities related issue is `bound set`.
Here we only setcap for `Permitted`, but the program change its Effective caps in method `main` to make `CAP_NET_RAW` and `CAP_NET_ADMIN` effective. 

