/*
 * =====================================================================================
 *
 *       Filename:  data_send.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  01/25/2013 04:19:02 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Billowkiller (bk), billowkiller@gmail.com
 *   Organization:
 *
 * =====================================================================================
 */
#include "data_send.h"

 inline u_short in_cksum(u_short *addr, int len)
{
    register int nleft = len;
    register u_short *w = addr;
    register int sum = 0;
    u_short answer = 0;

     /* Our algorithm is simple, using a 32 bit accumulator (sum), we add
      * sequential 16 bit words to it, and at the end, fold back all the
      * carry bits from the top 16 bits into the lower 16 bits. */

     while (nleft > 1) {
         sum += *w++;
         nleft -= 2;
     }

     /* mop up an odd byte, if necessary */
     if (nleft == 1) {
         *(u_char *)(&answer) = *(u_char *) w;
         sum += answer;
     }

     /* add back carry outs from top 16 bits to low 16 bits */
     sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
     sum += (sum >> 16); /* add carry */
     answer = ~sum; /* truncate to 16 bits */
     return(answer);
}



int make_pack(char *pack_msg, struct iphdr *iph, struct tcphdr *tcph, char *data, int datalen)
{
	struct pseudo_hdr * pseudo_h;
	pack_msg = (char *)malloc(ntohs(iph->tot_len));
	memcpy(pack_msg, iph, iph->ihl*4);

	//tcp checksum
	pseudo_h = (struct pseudo_hdr *)malloc(PSEUDO_SIZE);
	tcph->check = 0;
	pseudo_h->saddr = iph->saddr;
	pseudo_h->daddr = iph->daddr;
	pseudo_h->mbz = 0;
    pseudo_h->ptcl = IPPROTO_TCP;
    pseudo_h->tcpl = htons(ntohs(iph->tot_len) - iph->ihl*4);
    memcpy(&(pseudo_h->tcp), tcph, tcph->doff*4);

    u_short *checksum_data = (u_short *)malloc(PSEUDO_SIZE + datalen);
   	memcpy(checksum_data, pseudo_h, PSEUDO_SIZE);
   	memcpy(checksum_data + PSEUDO_SIZE, data, datalen);
   	tcph->check = in_cksum(checksum_data, PSEUDO_SIZE + datalen);
   	free(pseudo_h);
   	//end

	memcpy(pack_msg + iph->ihl*4, tcph, tcph->doff*4);
	memcpy(pack_msg + iph->ihl*4 + tcph->doff*4, data, datalen);

	return ntohs(iph->tot_len);
}

int send_alldata(Node * head)
{
	
	head = head->next;

	if(head == NULL)
		return;

	//set dest address
	bzero(&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = head->iphdr->daddr;
	sa.sin_port = ntohs(head->tcphdr->dest);

	if ((fd = socket(AF_INET, SOCK_RAW, 0)) < 0) {
		printf("create socket error!");
		return 0;
	}

	setsockopt(fd, IPPROTO_IP, IP_HDRINCL, (char*)&optval, sizeof(optval));

	
	while(head != NULL)
	{
		char * pack_msg;
		int length = make_pack(pack_msg, head->iphdr, head->tcphdr, head->payload, head->datalen);
		if(sendto(fd, pack_msg, length, 0, (struct sockaddr *)&sa, sizeof(sa))<0)
		{
			perror("tcp error");
		}
		free(pack_msg);
		head = head->next;
	}
	close(fd);	
	return 1;
}

int send_direct(struct iphdr *iph, struct tcphdr *tcph)
{
	//set dest address
	bzero(&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = iph->daddr;
	sa.sin_port = ntohs(tcph->dest);

	if ((fd = socket(AF_INET, SOCK_RAW, 0)) < 0) {
		printf("create socket error!");
		return 0;
	}

	setsockopt(fd, IPPROTO_IP, IP_HDRINCL, (char*)&optval, sizeof(optval));

	
	char * pack_msg = (char *)malloc(ntohs(iph->tot_len));;
	memcpy(pack_msg, iph, iph->ihl*4);
	memcpy(pack_msg + iph->ihl*4, tcph, tcph->doff*4);

	if(sendto(fd, pack_msg, ntohs(iph->tot_len), 0, (struct sockaddr *)&sa, sizeof(sa))<0)
	{
		perror("tcp error");
	}
	free(pack_msg);
	close(fd);	
	return 1;
}

//first hand shark
int send_rst(struct iphdr *iph, struct tcphdr *tcph)
{
	//set dest address
	bzero(&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = iph->daddr;
	sa.sin_port = ntohs(tcph->dest);

	char * pack_msg = (char *)malloc(TCPHL + IPHL);
	
	struct iphdr * this_iph = (struct iphdr *)pack_msg;
	memcpy(this_iph, iph, IPHL);
	this_iph->saddr = iph->daddr;
	this_iph->daddr = iph->saddr;

	struct tcphdr *this_tcph = (struct tcphdr *)(pack_msg + IPHL);
	memcpy(this_tcph, tcph, TCPHL);
	this_tcph->ack = 1;
	this_tcph->psh = 1;
	this_tcph->dest = tcph->source;
	this_tcph->source = tcph->dest;
	this_tcph->check = 0;

	//tcp checksum
	struct pseudo_hdr * pseudo_h = (struct pseudo_hdr *)malloc(PSEUDO_SIZE);
	pseudo_h->saddr = this_iph->saddr;
	pseudo_h->daddr = this_iph->daddr;
	pseudo_h->mbz = 0;
    pseudo_h->ptcl = IPPROTO_TCP;
    pseudo_h->tcpl = htons(TCPHL);
    memcpy(&(pseudo_h->tcp), this_tcph, TCPHL);

   	this_tcph->check = in_cksum((void *)pseudo_h, sizeof(struct pseudo_hdr));
   	free(pseudo_h);
   	//end

	if ((fd = socket(AF_INET, SOCK_RAW, 0)) < 0) {
		printf("create socket error!");
		return 0;
	}

	setsockopt(fd, IPPROTO_IP, IP_HDRINCL, (char*)&optval, sizeof(optval));

   	if(sendto(fd, pack_msg, TCPHL + IPHL, 0, (struct sockaddr *)&sa, sizeof(sa)) < 0)
	{
		perror("tcp:");
	}
	return 1;
}

