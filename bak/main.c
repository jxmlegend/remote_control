


static void parse_options(int argc, char *argv[])
{   
#if 0
    int ch;
    while((ch = getopt(argc, argv, "t:s:n:p:h")) != -1)
    {   
        switch(ch)
        {   
            case 't':
                if(strlen(optarg) > 8)
                {   
                    DEBUG("tftp ip:%s", optarg);
                    strcpy(conf.tftp_ip, optarg);
                }
                break;
            case 's':
                if(strlen(optarg) > 8)
                {   
                    DEBUG("server ip:%s", optarg);
                    strcpy(conf.server.ip, optarg);
                }
                break;
            case 'n':
                if(strlen(optarg) > 8)
                {   
                    DEBUG("nfs ip:%s", optarg);
                    strcpy(conf.nfs_ip, optarg);
                }
                break;                  
            case 'p':                   //httpd
                if(strlen(optarg) > 8)
                {   
                    DEBUG("http ip:%s", optarg);
                    strcpy(conf.http_ip, optarg);
                }
                break;
            case 'h':
                usage();
                break;
            default:
                break;
        }
    }
#endif
}

