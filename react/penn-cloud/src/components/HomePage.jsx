import { useState } from 'react';
import Cookies from 'js-cookie';
import NavBar from './NavBar';
import { Flex, ButtonGroup, Heading, VStack, Box, Alert, AlertIcon, AlertTitle } from "@chakra-ui/react";
import EmailPage from './EmailPage';
import AdminPage from './AdminPage';
import StoragePage from './StoragePage';

const HomePage = () => {
  const [service, setService] = useState(0);

  return (
    <Flex height={'100vh'} flexDir={'column'}>
      <NavBar username={Cookies.get('username')} service={service} setService={setService} />
      {service === 0 && <EmailPage />}
      {service === 1 && <StoragePage />}
      {service === 2 && <AdminPage />}
      
    </Flex>

  );
};

export default HomePage;