import { VStack, } from '@chakra-ui/layout';
import {
  Center,
  Heading,
  Stack,
  HStack,
  Divider,
  Switch,
  useColorModeValue,
  Table,
  Thead,
  Tbody,
  Alert,
  AlertIcon,
  AlertTitle,
  Tr,
  Th,
  Td,
  TableCaption,
  TableContainer,
} from '@chakra-ui/react';
import React, { useEffect, useState } from 'react';
import IsolatedModal from './IsolatedModal';

const AdminPage = () => {
  const [error, setError] = useState("");
  const [serverStatuses, setServerStatuses] = useState({});
  const [frontendServerStatuses, setFrontendServerStatuses] = useState({});
  const [serverRowPeeks, setServerRowPeeks] = useState({});

  useEffect(() => {
    getServerStatuses();
    getRowPreview();
    getFrontendServerStatuses();
    document.title = "PennCloud - Admin Console";
  }, []);

  const getFrontendServerStatuses = () => {
    var placeholderObj = {
      placeholder: 'value',
    };
    const url = `http://${window.location.host}/api/admin/get_http_server_status`;
    fetch(url, {
      method: 'POST',
      credentials: 'include',
      headers: {
        'Accept': 'application/json',
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(placeholderObj),
    }).then(response => {
      if (!response.ok) {
        console.log("Error with getting frontend server statuses");
      } else {
        response.json().then((responseJson) => {
          setFrontendServerStatuses(responseJson);
          console.log(responseJson);
        });
      }
    });
  };

  const getServerStatuses = () => {
    var placeholderObj = {
      placeholder: 'value',
    };
    const url = `http://${window.location.host}/api/admin/get_server_status`;
    fetch(url, {
      method: 'POST',
      credentials: 'include',
      headers: {
        'Accept': 'application/json',
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(placeholderObj),
    }).then(response => {
      if (!response.ok) {
        console.log("Error with getting server statuses");
      } else {
        response.json().then((responseJson) => {
          setServerStatuses(responseJson);
          console.log(responseJson);
        });
      }
    });
  };

  const getRowPreview = () => {
    var placeholderObj = {
      placeholder: 'value',
    };
    const url = `http://${window.location.host}/api/admin/retrieve_all_rows`;
    fetch(url, {
      method: 'POST',
      credentials: 'include',
      headers: {
        'Accept': 'application/json',
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(placeholderObj),
    }).then(response => {
      if (!response.ok) {
        console.log("Error with getting row preview");
      } else {
        response.json().then((responseJson) => {
          setServerRowPeeks(responseJson);
          console.log(responseJson);
        });
      }
    });
  };

  const handleChange = (event) => {
    const { name, checked } = event.target;
    const num_char = name.charAt(name.length - 1);
    let number = parseInt(num_char);
    console.log(`${number} is now ${checked ? 'on' : 'off'}`);

    var serverObj = {
      server: number,
      toggle: `${checked ? 'on' : 'off'}`,
    };

    //change to ${window.location.host}
    const url = `http://${window.location.host}/api/admin/toggle_server`;
    fetch(url, {
      method: 'POST',
      credentials: 'include',
      headers: {
        'Accept': 'application/json',
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(serverObj),
    }).then(response => {
      if (!response.ok) {
        response.json().then((responseJson) => {
          console.log(responseJson);
          setError(responseJson.message);
        });
      } else {
        response.json().then((responseJson) => {
          getServerStatuses();
          console.log(responseJson);
        });
      }
    });
    getServerStatuses();
  };

  return (
    <VStack w={{ base: "90%", md: "500px" }} m="auto" justify="center" h="100vh" spacing="1rem" fontSize="lg">
      {error.length > 0 && <Alert status='error' mt={3} mb={3}>
        <AlertIcon />
        <AlertTitle>{error}</AlertTitle>
      </Alert>}
      <Center py={6} justify="center" h="100vh" spacing="1rem">
        <HStack spacing="3rem" align="top">
          <VStack spacing="2rem">
            {/* <Heading> View HTTP Servers</Heading> */}
            <Stack
              borderWidth="1px" borderRadius="lg" w={{ sm: '100%', md: '540px' }} maxH="80vh"
              direction={{ base: 'column', md: 'row' }}
              bg={useColorModeValue('white', 'gray.900')} boxShadow={'2xl'} padding={4}>
              <Stack
                flex={1} flexDirection="column"
                justifyContent="center" alignItems="center" p={1} pt={2}>
                <Heading fontSize={'2xl'} fontFamily={'body'}>
                  HTTP Servers
                </Heading>
                <Divider />

                <TableContainer scrollBehavior="inside" overflowY="auto">
                  <Table variant='simple' size="lg">
                    <TableCaption>All available frontend servers</TableCaption>
                    <Thead>
                      <Tr>
                        <Th>HTTP Server</Th>
                        <Th>Current Status</Th>
                      </Tr>
                    </Thead>
                    <Tbody>
                      {Object.entries(frontendServerStatuses).map(([id, status]) => (
                        <Tr key={id}>
                          <Td>
                            Server {id}
                          </Td>
                          <Td paddingLeft={75}>{status ? 'Online' : 'Offline'}</Td>
                        </Tr>
                      ))}
                    </Tbody>
                  </Table>
                </TableContainer>
              </Stack>
            </Stack>
          </VStack>

          <VStack spacing="2rem">
            {/* <Heading> Toggle Storage Servers</Heading> */}
            <Stack
              borderWidth="1px" borderRadius="lg" w={{ sm: '100%', md: '540px' }} maxH="80vh"
              direction={{ base: 'column', md: 'row' }}
              bg={useColorModeValue('white', 'gray.900')} boxShadow={'2xl'} padding={4}>
              <Stack
                flex={1} flexDirection="column"
                justifyContent="center" alignItems="center" p={1} pt={2} scrollBehavior='inside'>
                <Heading fontSize={'2xl'} fontFamily={'body'}>
                  Storage Servers
                </Heading>
                <Divider />

                <TableContainer scrollBehavior="inside" overflowY="auto">
                  <Table variant='simple' size="lg">
                    <TableCaption>All available storage servers & clients</TableCaption>
                    <Thead>
                      <Tr>
                        <Th>Storage Server</Th>
                        <Th>Current Status</Th>
                      </Tr>
                    </Thead>
                    <Tbody>
                      {Object.entries(serverStatuses).map(([id, status]) => (
                        <Tr key={id}>
                          <Td>
                            Server {id}
                            <Switch name={`server${id}`} onChange={handleChange} paddingLeft={5} size='lg' checked={status} defaultChecked={status} />
                          </Td>
                          <Td paddingLeft={75}>{status ? 'Online' : 'Offline'}</Td>
                        </Tr>
                      ))}
                    </Tbody>
                  </Table>
                </TableContainer>
              </Stack>
            </Stack>
          </VStack>

          <VStack spacing="2rem">
            {/* <Heading> View Key-Value Store Data</Heading> */}
            <Stack
              borderWidth="1px" borderRadius="lg" w={{ sm: '100%', md: '540px' }} maxH="80vh"
              direction={{ base: 'column', md: 'row' }}
              bg={useColorModeValue('white', 'gray.900')} boxShadow={'2xl'} padding={4}>
              <Stack
                flex={1} flexDirection="column" overflowY="auto"
                justifyContent="center" alignItems="center" p={1} pt={2} scrollBehavior="inside">
                <Heading fontSize={'2xl'} fontFamily={'body'} >
                  Key-Value Store Data
                </Heading>
                <Divider />

                <TableContainer scrollBehavior="inside" overflowY="auto">
                  <Table variant='simple' size="lg" scrollBehavior="inside">
                    <TableCaption>All currently available rows. Click to view</TableCaption>
                    <Thead>
                      <Tr>
                        <Th>Row</Th>
                        <Th> Access Data </Th>
                      </Tr>
                    </Thead>
                    <Tbody>
                      {Object.entries(serverRowPeeks).map(([id, out_values]) => (
                        <Tr key={id}>
                          <Td>
                            {id}
                          </Td>
                          <IsolatedModal key={id} row_id={id} values_passed={out_values} />
                        </Tr>
                      ))}
                    </Tbody>
                  </Table>
                </TableContainer>
              </Stack>
            </Stack>
          </VStack>
        </HStack>

      </Center>
    </VStack>
  );
};

export default AdminPage;;