import { ReactNode } from 'react';
import {
  Box,
  Flex,
  Avatar,
  Link,
  Button,
  Menu,
  MenuButton,
  MenuList,
  MenuItem,
  MenuDivider,
  useDisclosure,
  useColorModeValue,
  Stack,
  useColorMode,
  Center,
  Text,
  Image,
  useToast,
} from '@chakra-ui/react';
import { ChevronDownIcon } from '@chakra-ui/icons'
import { MoonIcon, SunIcon } from '@chakra-ui/icons';
import { useNavigate } from "react-router";

const NavBar = ({
	username,
	service, 
	setService
}) => {
  const navigate = useNavigate();
  const toast = useToast();
  const { colorMode, toggleColorMode } = useColorMode();
  let serviceName;
  let serviceLogoPath;
  switch (service) {
	case 0:
		serviceName = "Email";
		serviceLogoPath = "https://i.imgur.com/NgWA2gG.png";
		break;
	case 1:
		serviceName = "Storage";
		serviceLogoPath = "https://i.imgur.com/6wVpIfe.png";
		break;
	case 2:
		serviceName = "Admin";
		serviceLogoPath = "https://i.imgur.com/9Gbmy4m.png";
		break;
  }

  // const onLogout = () => {
  //   fetch(`http://${window.location.host}/api/account/logout`, {
  //     method: 'POST',
  //     credentials: 'include',
  //     headers: {
  //       'Accept': 'application/json',
  //       'Content-Type': 'application/json',
  //     }
  //   }).then(response => {
  //     if (!response.ok) {
  //       toast({
  //         title: 'Error',
  //         description: "Failed to logout",
  //         status: 'error',
  //         duration: 3000,
  //         isClosable: true,
  //         position: 'top-center',
  //       });
  //     } else {
  //       response.json().then((responseJson) => {
  //         navigate("/login");
  //       });
  //     }
  //   });
  // };
  
  return (
    <>
      <Box bg={useColorModeValue('gray.100', 'gray.900')} px={4}>
        <Flex h={16} alignItems={'center'} justifyContent={'space-between'}>
          <Flex  ml={3} alignItems={'center'}>
				<Image src={serviceLogoPath} height={30} m={0}></Image>
				<Menu p={0}>
					<MenuButton p={0} ml={2} as={Button} rightIcon={<ChevronDownIcon />}>
						<Text as={'span'} fontSize={20} fontWeight={'bold'} pl={3}>{serviceName}</Text>
					</MenuButton>
					<MenuList>
						<MenuItem onClick={() => setService(0)}>Email</MenuItem>
						<MenuItem onClick={() => setService(1)}>Storage</MenuItem>
						<MenuItem onClick={() => setService(2)}>Admin</MenuItem>
					</MenuList>
					</Menu>
			</Flex>

          <Flex alignItems={'center'}>
            <Stack direction={'row'} spacing={7}>
              <Button onClick={toggleColorMode}>
                {colorMode === 'light' ? <MoonIcon /> : <SunIcon />}
              </Button>

              <Menu>
                <MenuButton
                  as={Button}
                  rounded={'full'}
                  variant={'link'}
                  cursor={'pointer'}
                  minW={0}>
                  <Avatar
                    size={'sm'}
                    src={'https://api.dicebear.com/6.x/avataaars/svg?seed=Garfield'}
                  />
                </MenuButton>
                <MenuList alignItems={'center'}>
                  <br />
                  <Center>
                    <Avatar
                      size={'2xl'}
                      src={'https://api.dicebear.com/6.x/avataaars/svg?seed=Garfield'}
                    />
                  </Center>
                  <br />
                  <Center>
                    <Text as={'p'} fontWeight='semibold'>{username}</Text>
                  </Center>
                  <br />
                  <MenuDivider />
                  {/* <MenuItem onClick={onLogout}>Logout</MenuItem> */}
                </MenuList>
              </Menu>
            </Stack>
          </Flex>
        </Flex>
      </Box>
    </>
  );
}

export default NavBar;