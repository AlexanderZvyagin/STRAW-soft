float F_CARD_ID(float chf,float pos,char chamber)
{
 if(chamber == 'X' ||chamber == 'U' || chamber == 'V')
 {
    switch(chf)
        {
        case 0:
                return 0;
        case 32:
                return 1;
        case 64:
                return 2;
        case 95:
                switch(pos)
                {
                    case -1:
                        return 3;
                    case 1:
                        return 7;
                }
        case 127:
                return 4;
        case 158:
                return 5;
        case 190:
                return 6;
            printf("No card could be identified with chf = %f \n",chf);
            break;
        }
    return 1;
 }
 else if(chamber == 'Y')
 {
    
    switch(chf)
        {
        case 0:
                return 0;
        case 32:
                return 1;
        case 64:
                return 2;
        case 80:
//                 switch(pos)
//                 {
//                     case -1:
//                         return 2;
//                     case 1:
                         return 6;
//                 }
        case 96:
//                 switch(pos)
//                 {
//                     case -1:
                         return 3;
//                     case 1:
//                         return 6;
//                 }
                
        case 112:
                return 3;
        case 128:
                return 4;
        case 160:
                return 5;
                
        }
    
    
 }
 else printf("!!!!!!!!! (in F_CARD_ID) Chamber : %c is not a posible chamber ",chmbr);  


}
