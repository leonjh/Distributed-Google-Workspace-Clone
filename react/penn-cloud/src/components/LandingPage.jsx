import React from 'react';
import { useNavigate } from "react-router";
import {
    Box,
    Button,
    Center,
    Flex,
    Heading,
    Image,
    Stack,
    HStack,
    Text,
    useColorModeValue,
} from '@chakra-ui/react';
// import { Bade, Link } from '@chakra-ui/react';



export const LandingPage = () => {

    const navigate = useNavigate();
    return (
        <Center py={6} justify="center" h="100vh" spacing="1rem">
            <HStack spacing="2rem">
                <Stack
                    as={Box}
                    align={'left'} justify={'left'} direction={'column'}
                    spacing={{ base: 8, md: 14 }}
                    py={{ base: 20, md: 36 }}>
                    <Heading
                        fontWeight={600}
                        fontSize={{ base: '2xl', sm: '4xl', md: '6xl' }}
                        lineHeight={'110%'}>
                        Welcome to <br />
                        <Text as={'span'} color={'blue.600'}>
                            Penn
                        </Text>
                        <Text as={'span'} color={'red.600'}>
                            Cloud
                        </Text>
                    </Heading>
                    <Text color={'gray.500'} fontSize='2xl' noOfLines={3}>
                        Penn's premier student-made <br /> cloud hosting service.
                    </Text>
                </Stack>
                <Stack
                    borderWidth="1px"
                    borderRadius="lg"
                    w={{ sm: '100%', md: '540px' }}
                    height={{ sm: '476px', md: '20rem' }}
                    direction={{ base: 'column', md: 'row' }}
                    bg={useColorModeValue('white', 'gray.900')}
                    boxShadow={'2xl'}
                    padding={4}>
                    <Flex flex={1} bg="blue.200">
                        <Image
                            objectFit="cover"
                            boxSize="100%"
                            src={
                                'https://www.cis.upenn.edu/wp-content/uploads/2019/08/Levine-west-facade.jpg'
                            }
                        />
                    </Flex>
                    <Stack
                        flex={1}
                        flexDirection="column"
                        justifyContent="center"
                        alignItems="center"
                        p={1}
                        pt={2}>
                        <Heading fontSize={'2xl'} fontFamily={'body'}>
                            PennCloud
                        </Heading>
                        <Text
                            textAlign={'left'}
                            color={useColorModeValue('gray.700', 'gray.400')}
                            px={3}>
                            {/* <Link href={'#'} color={'blue.400'}>
                                #tag
                            </Link> */}
                            Equipped with an email client, file hosting, and more, PennCloud
                            is the ideal tool for Penn students to safely store their data.
                        </Text>
                        <Stack align={'left'} justify={'left'} direction={'column'} mt={6}>
                            {/* <Badge
                                px={2}
                                py={1}
                                bg={useColorModeValue('gray.50', 'gray.800')}
                                fontWeight={'400'}>
                                #art
                            </Badge>
                            <Badge
                                px={2}
                                py={1}
                                bg={useColorModeValue('gray.50', 'gray.800')}
                                fontWeight={'400'}>
                                #photography
                            </Badge>
                            <Badge
                                px={2}
                                py={1}
                                bg={useColorModeValue('gray.50', 'gray.800')}
                                fontWeight={'400'}>
                                #music
                            </Badge> */}
                        </Stack>
                        <Stack
                            width={'100%'}
                            mt={'2rem'}
                            direction={'row'}
                            padding={2}
                            justifyContent={'space-between'}
                            alignItems={'center'}>
                            <Button
                                flex={1}
                                fontSize={'sm'}
                                rounded={'full'}
                                bg={'blue.400'}
                                color={'white'}
                                boxShadow={'0px 1px 25px -5px rgb(66 153 225 / 48%), 0 10px 10px -5px rgb(66 153 225 / 43%)'}
                                _hover={{ bg: 'blue.500', }}
                                _focus={{ bg: 'blue.500', }}
                                onClick={() => navigate("/login")}>
                                Get Started
                            </Button>
                            <Button
                                flex={1}
                                fontSize={'sm'}
                                rounded={'full'}
                                _focus={{ bg: 'gray.200', }}>
                                Learn More
                            </Button>
                        </Stack>
                    </Stack>
                </Stack>
            </HStack>

        </Center>
    );
};

export default LandingPage;