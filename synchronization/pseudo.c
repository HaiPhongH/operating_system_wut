// Init 5 semaphores
int barbers_for_woman = N1;
int barbers_for_man = N2;
int barbers_for_both = N3;
int customers_available = 0;
int chairs_available = M;

Barber:
    wait(customers_available);      // wait for customer to show up
   
    /*  do haircut   */
    if customer is a woman && barbers_for_woman < N1:
        sleep(5);
        signal(barbers_for_woman);  // after one customer has been served, barber for woman is free
    else if customer is a man && barbers_for_man < N2:
        sleep(5);
        signal(barbers_for_man);  // after one customer has been served, barber for man is free
    else:
        sleep(5);
        signal(barbers_for_both);  // after one customer has been served, barber for both is free
    endif


Customer:
    sleep(1);
    wait(chairs_available);         // wait for an available chair
    if chairs_available > 0:
        signal(customers_available);    // once customer sit, signal to the barber that there's customers waiting
        
        if customer is a woman && barbers_for_woman > 0:
            wait(barbers_for_woman);    // wait for one of the barbers for woman to be free
        else if customer is a man && barbers_for_man > 0:
            wait(barbers_for_man);      // wait for one of the barbers for man to be free
        else:
            wait(barbers_for_both);     // wait for one of the barbers for both to be free
        endif
      
        signal(chairs_available);       // once it's your turn,  go to barber and free a chair
        /*  get haircut  */
    else:
        // customer leave
    endif